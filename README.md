# COF33 


COFF33 is a quick and dirty COFF loader. It can be used to run COFF objects. In order to showcase the COFF Loader, the `bamsi.c` file is also provided that implements a malicious technique to bypass AMSI through byte patching. COFF33 is not limited to this atttack.  

If insterested of how this actually works visit my <a href="https://xen0vas.github.io/Make-a-COFFee-relax-and-bypass-AMSI/">blog</a> where I have described the COFF in detail. 

### Disclaimer 

Use with your own risk

## Compile and run 

open a powershell session and compile the COFF Loader as well as the object file

```
.\compile.bat .\COFFee.c
```

Then run 

```
.\coffee.exe .\bamsi.o
```

At this point AMSI is bypassed and suites and tools such as PowerSploit that are flagged by AV engines can now run undetected. 



