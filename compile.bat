@ECHO OFF
cl.exe /nologo /Ox /MT /W0 /GS- /DNDEBUG /Tc COFFee.c  /link /OUT:COFFee.exe /SUBSYSTEM:CONSOLE

cl.exe /nologo /c /Od /MT /W0 /GS- /Tc bypassamsi.c
move /y bypassamsi.obj bypassamsi.o
del *.obj
