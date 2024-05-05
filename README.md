# COF33 


COFF33 is a quick and dirty COFF loader. It can be used to run COFF objects. In order to showcase the COFF Loader, the `bamsi.c` file is also provided that implements a malicious technique to bypass AMSI through byte patching. If insterested of how this actually works visit my <a href="https://xen0vas.github.io/Make-a-COFFee-relax-and-bypass-AMSI/">blog</a>
where I have described the COFF in detail. 

## Compile and run 

compile the COFF Loader and the object file as follows 

```
.\compile.bat .\COFFee.c
```

open a powershell session and run the following 

```
.\coffee.exe .\bamsi.o
```

At this point AMSI is bypassed and suites and tools such as PowerSploit that are flagged by AV engines can now run undetected. 



