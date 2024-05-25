# COFF33 


COFF33 is a quick and dirty COFF loader. It can be used to run COFF objects. In order to showcase the COFF Loader, the `bamsi.c` file is also provided that implements AMSI bypass through byte patching.  

If insterested of how a COFF Loader actually works visit my <a href="https://xen0vas.github.io/Make-a-COFFee-relax-and-bypass-AMSI/">blog</a> where I have described the COFF in detail. 

## Disclaimer 

For educational purposes only.

## Compile and run 

open a powershell session and compile the COFF Loader as well as the object file

```
.\compile.bat .\COFFee.c
```

Then run 

```
.\coffee.exe .\bamsi.o
```





