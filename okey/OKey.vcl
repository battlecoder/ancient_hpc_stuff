<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: OKey - Win32 (WCE x86) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating command line "rc.exe /l 0x409 /fo"X86Rel/resources.res" /d UNDER_CE=300 /d _WIN32_WCE=300 /d "UNICODE" /d "_UNICODE" /d "NDEBUG" /d "WIN32_PLATFORM_HPC2000" /d "_X86_" /d "x86" /d "_i386_" /r "D:\15\projects\okey\resources.rc"" 
Creating temporary file "C:\WINDOWS\TEMP\RSP82A1.TMP" with contents
[
/nologo /W3 /D _WIN32_WCE=300 /D "WIN32_PLATFORM_HPC2000" /D "_i386_" /D UNDER_CE=300 /D "i_386_" /D "UNICODE" /D "_UNICODE" /D "_X86_" /D "x86" /D "NDEBUG" /Fp"X86Rel/OKey.pch" /YX /Fo"X86Rel/" /Gs8192 /GF /Oxs /c 
"D:\15\projects\okey\OKey.c"
"D:\15\projects\okey\StdAfx.cpp"
]
Creating command line "cl.exe @C:\WINDOWS\TEMP\RSP82A1.TMP" 
Creating temporary file "C:\WINDOWS\TEMP\RSP82A2.TMP" with contents
[
commctrl.lib coredll.lib corelibc.lib /nologo /base:"0x00010000" /stack:0x10000,0x1000 /entry:"WinMainCRTStartup" /incremental:no /pdb:"X86/OKey.pdb" /nodefaultlib:"OLDNAMES.lib" /nodefaultlib:libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /nodefaultlib:oldnames.lib /out:"X86/OKey.exe" /subsystem:windowsce,3.00 /MACHINE:IX86 
.\X86Rel\OKey.obj
.\X86Rel\StdAfx.obj
.\X86Rel\resources.res
]
Creating command line "link.exe @C:\WINDOWS\TEMP\RSP82A2.TMP"
<h3>Output Window</h3>
Compiling resources...
Compiling...
OKey.c
StdAfx.cpp
Linking...




<h3>Results</h3>
OKey.exe - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
