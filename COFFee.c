#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <strsafe.h>
#include <shlwapi.h>

#define CopyMem   __movsb

#define MEM_SYMNAME_MAX		100

#pragma comment(lib, "Shlwapi.lib")

#define IMAGE_REL_AMD64_ADDR32NB    0x0003
#define IMAGE_REL_AMD64_REL32       0x0004

typedef struct _COFF_FILE_HEADER {
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} COFF_FILE_HEADER;


#pragma pack(push,1)
/* Size of 40 */
typedef struct _COFF_SECTION {
    char Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLineNumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
} COFF_SECTION;


/* size of 10 */
typedef struct _COFF_RELOCATION {
    uint32_t VirtualAddress;
    uint32_t SymbolTableIndex;
    uint16_t Type;
} COFF_RELOCATION;


/* size of 18 */
typedef struct _COFF_SYMBOL {
    union {
        char ShortName[8];
		struct {
			uint32_t Zeros;
			uint32_t Offset;
		};
    } first;
    uint32_t Value;
    uint16_t SectionNumber;
    uint16_t Type;
    uint8_t StorageClass;
    uint8_t NumberOfAuxSymbols;
} COFF_SYMBOL;


typedef struct _COFF_MEM_SECTION {
	uint32_t	Counter;				
	char		Name[10];				
	uint32_t	SizeOfRawData;			
	uint32_t	PointerToRawData;		
	uint32_t	PointerToRelocations;	
	uint16_t	NumberOfRelocations;	
	uint32_t	Characteristics;		
	uint64_t	InMemoryAddress;		
	uint32_t	InMemorySize;			
} COFF_MEM_SECTION;


typedef struct _COFF_SYM_ADDR {
	uint32_t	Counter;				
	char		Name[MEM_SYMNAME_MAX];	
	uint16_t	SectionNumber;			
	uint32_t	Value;					
	uint8_t		StorageClass;			
	uint64_t	InMemoryAddress;		
	uint64_t	GOTaddress;				
} COFF_SYM_ADDR;
#pragma pack(pop)

COFF_MEM_SECTION * 	memSections 		= NULL;
COFF_SYM_ADDR * 	memSymbols 		    = NULL;
int 				memSections_size 	= 0;
int 				memSymbols_size 	= 0;
char * 				iGOT				= NULL;
int 				iGOT_index			= 0;
void 				(* hitTheGoFunction)(void);

int InMemoryResolveSymbols(void) {
	char * symbol= NULL;
	char * DLL = NULL;
	char * FName = NULL;
	int section = 0;
	
	for (int i = 0; i < memSymbols_size ; i++) {
		memSymbols[i].GOTaddress = NULL;
		symbol = malloc(sizeof(char) * memSymbols_size);
		if (symbol == NULL) {
			return -1;
		}
		memset(symbol, 0, sizeof(char) * memSymbols_size);
		CopyMem(symbol, memSymbols[i].Name, strlen(memSymbols[i].Name));
		
		if (StrStrIA(symbol, "__imp_")) {
			FName = strchr(symbol, '$'); 
			if (FName != NULL) {
				DLL = symbol + strlen("__imp_");
				strtok_s(symbol, "$", &FName);
			}
			
			HANDLE lib = LoadLibraryA(DLL);
			if (lib != NULL) {
				memSymbols[i].InMemoryAddress = GetProcAddress(lib, FName);
				CopyMem(iGOT + (iGOT_index * 8), &memSymbols[i].InMemoryAddress, sizeof(uint64_t));
				memSymbols[i].GOTaddress = iGOT + (iGOT_index * 8);
				iGOT_index++;
			}
		}
		else {
			section = memSymbols[i].SectionNumber - 1;
			memSymbols[i].InMemoryAddress = memSections[section].InMemoryAddress + memSymbols[i].Value;
			if (!strncmp(symbol, "go", 3)) {
				hitTheGoFunction = memSymbols[i].InMemoryAddress;
			}		
		}
		free(symbol);		
	}

	return 0;
}

int LoadTheCOFFObject(unsigned char * COFF_data) {
    COFF_FILE_HEADER * 	header_ptr = NULL;
    COFF_SECTION *		sect_ptr = NULL;
    COFF_RELOCATION * 	reloc_ptr = NULL;
    COFF_SYMBOL * 		sym_ptr = NULL;
	
	header_ptr = (COFF_FILE_HEADER *) COFF_data;
	memSections_size = header_ptr->NumberOfSections;
	size_t MemSectionsSize = sizeof(COFF_MEM_SECTION) * memSections_size;
	memSections = calloc(memSections_size, sizeof(COFF_MEM_SECTION));
	if (!memSections) {
		return -1;
	}
	
	for (int i = 0 ; i < memSections_size ; i++) {
		sect_ptr = (COFF_SECTION *)(COFF_data + sizeof(COFF_FILE_HEADER) + (sizeof(COFF_SECTION) * i));
		if (sect_ptr->SizeOfRawData > 0) {
			memSections[i].Counter = i;
			strcpy_s(memSections[i].Name, strlen(sect_ptr->Name) + 1, sect_ptr->Name);
			memSections[i].Name[8] = '\0';
			memSections[i].SizeOfRawData = sect_ptr->SizeOfRawData;
			memSections[i].PointerToRawData = sect_ptr->PointerToRawData;
			memSections[i].PointerToRelocations = sect_ptr->PointerToRelocations;
			memSections[i].NumberOfRelocations = sect_ptr->NumberOfRelocations;
			memSections[i].Characteristics = sect_ptr->Characteristics;
			memSections[i].InMemorySize = memSections[i].SizeOfRawData  + (0x1000 - memSections[i].SizeOfRawData % 0x1000);
			memSections[i].InMemoryAddress = VirtualAlloc(NULL,memSections[i].InMemorySize,MEM_COMMIT | MEM_TOP_DOWN,(sect_ptr->Characteristics & IMAGE_SCN_CNT_CODE) ? PAGE_EXECUTE_READWRITE: PAGE_READWRITE);
			if (memSections[i].InMemoryAddress == NULL) {
				return -1;
			}

			if (sect_ptr->PointerToRawData > 0)
				CopyMem(memSections[i].InMemoryAddress, COFF_data + sect_ptr->PointerToRawData, sect_ptr->SizeOfRawData);
		}
	}
	
	memSymbols_size = header_ptr->NumberOfSymbols;
	memSymbols = calloc(memSymbols_size, sizeof(COFF_SYM_ADDR));
	
	if (!memSymbols) {
		return -1;
	}
		
	sym_ptr = (COFF_SYMBOL *) (COFF_data + header_ptr->PointerToSymbolTable);
	
	char * 	strings_ptr = (char *)((COFF_data + header_ptr->PointerToSymbolTable) + memSymbols_size * sizeof(COFF_SYMBOL));

	for (int i = 0 ; i < memSymbols_size ; i++) {

		if (sym_ptr[i].SectionNumber == 0 && sym_ptr[i].StorageClass == 0) {	

			strcpy_s(memSymbols[i].Name, MEM_SYMNAME_MAX, "__UNDEFINED");
		}
		else
		if (sym_ptr[i].first.Zeros != 0) {	
			char n[10];								
			strcpy_s(n, strlen(sym_ptr[i].first.ShortName) + 1, sym_ptr[i].first.ShortName);
			n[8] = '\0';
			strcpy_s(memSymbols[i].Name, MEM_SYMNAME_MAX, n);
		}
		else {
			strcpy_s(memSymbols[i].Name, MEM_SYMNAME_MAX, (char *)(strings_ptr + sym_ptr[i].first.Offset));
		}
		
		memSymbols[i].Counter = i;
		memSymbols[i].SectionNumber = sym_ptr[i].SectionNumber;
		memSymbols[i].Value = sym_ptr[i].Value;
		memSymbols[i].StorageClass = sym_ptr[i].StorageClass;
		memSymbols[i].InMemoryAddress = NULL;
	}
		
	iGOT = VirtualAlloc(NULL, 2048, MEM_COMMIT | MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);
	int ret = InMemoryResolveSymbols();
	if (ret != 0)
		return -1;
	
	int32_t relative_address = 0;
	char* rva = NULL;		
	int32_t reference_address = 0;
	int32_t offset32 = 0;

	for (int i = 0 ; i < memSections_size ; i++ ) {
		if (memSections[i].NumberOfRelocations != 0)
			for (int j = 0 ; j < memSections[i].NumberOfRelocations ; j++ ) {
				reloc_ptr = (COFF_RELOCATION *) (COFF_data + memSections[i].PointerToRelocations + sizeof(COFF_RELOCATION) * j);
				rva = NULL; 
				
					if ( reloc_ptr->Type == IMAGE_REL_AMD64_ADDR32NB ) { // Type 0x3
						rva = memSections[i].InMemoryAddress + reloc_ptr->VirtualAddress;
						CopyMem(&offset32, rva, sizeof(int32_t));
						relative_address = (memSymbols[reloc_ptr->SymbolTableIndex].InMemoryAddress) - ((int32_t) rva + 4);
						reference_address = offset32 + relative_address;
						CopyMem(rva, &reference_address, sizeof(uint32_t));
					}
					
					if ( reloc_ptr->Type == IMAGE_REL_AMD64_REL32 ) {  // Type 0x4
						rva = memSections[i].InMemoryAddress + reloc_ptr->VirtualAddress;
						CopyMem(&offset32, rva, sizeof(int32_t));
						if (memSymbols[reloc_ptr->SymbolTableIndex].GOTaddress != NULL)
							reference_address = (int32_t)((memSymbols[reloc_ptr->SymbolTableIndex].GOTaddress) - ((int32_t) rva + 4));
						else
						{
							relative_address = (memSymbols[reloc_ptr->SymbolTableIndex].InMemoryAddress) - ((int32_t) rva + 4);
							reference_address = offset32 + relative_address;
						}
						CopyMem(rva, &reference_address, sizeof(uint32_t));
					}
				}
	}

	if (hitTheGoFunction != NULL)
		hitTheGoFunction();
	else {
		return -1;
	}

	for (int i = 0 ; i < memSections_size ; i++)
		VirtualFree(memSections[i].InMemoryAddress, 0, MEM_RELEASE);
	
	VirtualFree(memSections, 0, MEM_RELEASE);
	VirtualFree(memSymbols, 0, MEM_RELEASE);
	VirtualFree(iGOT, 0, MEM_RELEASE);
	return 0;
}

int main(int argc, char * argv[]) {
	
	if (argc < 2) {
		printf("[!] Usage: %s <The_Path_Of_The_Second_File>\n", argv[0]);
		return -1;
	}
	
	HANDLE COFFfile = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (COFFfile == INVALID_HANDLE_VALUE) {
			printf("[x] Could not open file: %s (%#x)\n", argv[1], GetLastError());
			return -1;
	}

	HANDLE FileMapping = CreateFileMapping(COFFfile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (FileMapping == NULL) {
			return -1;
	}
	
	LPVOID COFF_data = MapViewOfFile(FileMapping, FILE_MAP_READ, 0, 0, 0);
	
	if (COFF_data == NULL) {
			return -1;
	}

	LoadTheCOFFObject((unsigned char *) COFF_data);
	
	UnmapViewOfFile(COFF_data);
	CloseHandle(FileMapping);
	CloseHandle(COFFfile);
	
	return 0;
}
