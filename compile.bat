@ECHO OFF
cl.exe /nologo /Ox /MT /W0 /GS- /DNDEBUG /Tc COFFee.c  /link /OUT:coffee.exe /SUBSYSTEM:CONSOLE

cl.exe /nologo /c /Od /MT /W0 /GS- /Tc bamsi.c
move /y bamsi.obj bamsi.o
del *.obj
