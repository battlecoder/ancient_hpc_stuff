<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: mouser - Win32 (WCE ARM) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\WINDOWS\TEMP\RSPC2D3.TMP" with contents
[
/nologo /W3 /O2 /D _WIN32_WCE=211 /D "WIN32_PLATFORM_HPCPRO" /D "ARM" /D "_ARM_" /D UNDER_CE=211 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /Fp"ARMRel/mouser.pch" /YX /Fo"ARMRel/" /Oxs /MC /c 
"D:\15\projects\mouser\mouser.c"
]
Creating command line "clarm.exe @C:\WINDOWS\TEMP\RSPC2D3.TMP" 
Creating temporary file "C:\WINDOWS\TEMP\RSPC2D4.TMP" with contents
[
commctrl.lib coredll.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"ARM/mouser.pdb" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /nodefaultlib:oldnames.lib" /out:"ARM/mouser.exe" /subsystem:windowsce,2.11 /align:"4096" /MACHINE:ARM 
.\ARMRel\mouser.obj
.\ARMRel\StdAfx.obj
.\ARMRel\app.res
]
Creating command line "link.exe @C:\WINDOWS\TEMP\RSPC2D4.TMP"
<h3>Output Window</h3>
Compiling...
mouser.c
Linking...



<h3>Results</h3>
mouser.exe - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
