# COF33 

COFF33 is a quick and dirty COFF loader. It can be used to run COFF objects. In order to showcase the COFF Loader the `bypassamsi.c` file is provided that implements a malicious technique to bypass AMSI through byte patching.  

## Compile and run 

compile the COFF Loader and the object file as follows 

```
.\compile.bat .\COFFee.c
```

open a powershell session and run the following 

```
.\COFFee.exe bypassamsi.o
```
At this point AMSI is bypassed and flagged tools by AV engines can run undetected. 
