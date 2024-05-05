# COF33 


COFF33 is a quick and dirty COFF loader. It can be used to run COFF objects. In order to showcase the COFF Loader, the `bypassamsi.c` file is also provided that implements a malicious technique to bypass AMSI through byte patching. If insterested of how this actually works visit my blog where I have described the COFF mechanism in detail. 

## Compile and run 

compile the COFF Loader and the object file as follows 

```
.\compile.bat .\COFFee.c
```

open a powershell session and run the following 

```
.\COFFee.exe bypassamsi.o
```

At this point AMSI is bypassed and tools such as Powerview that are flagged by AV engines can now run undetected. 



