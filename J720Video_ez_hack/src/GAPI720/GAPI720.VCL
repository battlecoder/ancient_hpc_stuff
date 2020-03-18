<html>
<body>
<pre>
<h1>Build Log</h1>
<h3>
--------------------Configuration: GAPI720 - Win32 (WCE ARM) Release--------------------
</h3>
<h3>Command Lines</h3>
Creating temporary file "C:\WINDOWS\TEMP\RSP184.TMP" with contents
[
/nologo /W3 /I "src" /I "src\LibLCD" /I "src\GAPI720" /I "..\LibLCD" /I "..\GAPI720" /D _WIN32_WCE=300 /D "WIN32_PLATFORM_HPC2000" /D "ARM" /D "_ARM_" /D UNDER_CE=300 /D "UNICODE" /D "_UNICODE" /D "NDEBUG" /D "_USRDLL" /D "GAPI720_EXPORTS" /Fp"..\..\bin\ARMRel\GAPI720\obj/GAPI720.pch" /YX /Fo"..\..\bin\ARMRel\GAPI720\obj/" /Oxs /MC /c 
"C:\Windows\Escritorio\J720Video\src\GAPI720\gx.cpp"
]
Creating command line "clarm.exe @C:\WINDOWS\TEMP\RSP184.TMP" 
Creating temporary file "C:\WINDOWS\TEMP\RSP0185.TMP" with contents
[
commctrl.lib coredll.lib /nologo /base:"0x00100000" /stack:0x10000,0x1000 /entry:"_DllMainCRTStartup" /dll /incremental:no /pdb:"..\..\bin\ARMRel\GAPI720/gx.pdb" /nodefaultlib:"libc.lib /nodefaultlib:libcd.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrt.lib /nodefaultlib:msvcrtd.lib /nodefaultlib:oldnames.lib" /out:"..\..\bin\ARMRel\GAPI720/gx.dll" /implib:"..\..\bin\ARMRel\GAPI720/gx.lib" /subsystem:windowsce,3.00 /align:"4096" /MACHINE:ARM 
\Windows\Escritorio\J720Video\bin\ARMRel\GAPI720\obj\gx.obj
\Windows\Escritorio\J720Video\bin\ARMRel\GAPI720\obj\version.res
\Windows\Escritorio\J720Video\bin\ARMRel\LibLCD\LibLCD.lib
]
Creating command line "link.exe @C:\WINDOWS\TEMP\RSP0185.TMP"
<h3>Output Window</h3>
Compiling...
gx.cpp
Linking...
   Creating library ..\..\bin\ARMRel\GAPI720/gx.lib and object ..\..\bin\ARMRel\GAPI720/gx.exp



<h3>Results</h3>
gx.dll - 0 error(s), 0 warning(s)
</pre>
</body>
</html>
