# Microsoft Developer Studio Project File - Name="msged.c" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=msged.c - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "msged.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "msged.mak" CFG="msged.c - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "msged.c - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "msged.c - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "msged.c - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\nd_r\bin"
# PROP Intermediate_Dir "..\nd_r\obj\msged.c"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "__NT__" /D "_MAKE_DLL" /D "INTEL" /D "USE_MSGAPI" /FR /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 msvcrt.lib Kernel32.lib user32.lib smapimvc.lib fconfmvc.lib /nologo /subsystem:console /machine:I386 /nodefaultlib /libpath:"..\nd_r\lib"

!ELSEIF  "$(CFG)" == "msged.c - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\nd_d\bin"
# PROP Intermediate_Dir "..\nd_d\obj\msged.c"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "__NT__" /D "INTEL" /D "USE_MSGAPI" /D "_MAKE_DLL" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcrtd.lib Kernel32.lib user32.lib smapimvc.lib fconfmvc.lib /nologo /subsystem:console /profile /debug /machine:I386 /nodefaultlib /libpath:"..\nd_d\lib"

!ENDIF 

# Begin Target

# Name "msged.c - Win32 Release"
# Name "msged.c - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\addr.c
# End Source File
# Begin Source File

SOURCE=.\areas.c
# End Source File
# Begin Source File

SOURCE=.\bmg.c
# End Source File
# Begin Source File

SOURCE=.\charset.c
# End Source File
# Begin Source File

SOURCE=.\config.c
# End Source File
# Begin Source File

SOURCE=.\control.c
# End Source File
# Begin Source File

SOURCE=.\date.c
# End Source File
# Begin Source File

SOURCE=.\dialogs.c
# End Source File
# Begin Source File

SOURCE=.\dirute.c
# End Source File
# Begin Source File

SOURCE=.\dlgbox.c
# End Source File
# Begin Source File

SOURCE=.\dlist.c
# End Source File
# Begin Source File

SOURCE=.\echotoss.c
# End Source File
# Begin Source File

SOURCE=.\environ.c
# End Source File
# Begin Source File

SOURCE=.\fconf.c
# End Source File
# Begin Source File

SOURCE=.\fecfg145.c
# End Source File
# Begin Source File

SOURCE=.\fido.c
# End Source File
# Begin Source File

SOURCE=.\filedlg.c
# End Source File
# Begin Source File

SOURCE=.\flags.c
# End Source File
# Begin Source File

SOURCE=.\freq.c
# End Source File
# Begin Source File

SOURCE=.\gestr120.c
# End Source File
# Begin Source File

SOURCE=.\getopts.c
# End Source File
# Begin Source File

SOURCE=.\group.c
# End Source File
# Begin Source File

SOURCE=.\help.c
# End Source File
# Begin Source File

SOURCE=.\helpcmp.c
# End Source File
# Begin Source File

SOURCE=.\helpinfo.c
# End Source File
# Begin Source File

SOURCE=.\init.c
# End Source File
# Begin Source File

SOURCE=.\keycode.c
# End Source File
# Begin Source File

SOURCE=.\list.c
# End Source File
# Begin Source File

SOURCE=.\maintmsg.c
# End Source File
# Begin Source File

SOURCE=.\makemsgn.c
# End Source File
# Begin Source File

SOURCE=.\memextra.c
# End Source File
# Begin Source File

SOURCE=.\menu.c
# End Source File
# Begin Source File

SOURCE=.\misc.c
# End Source File
# Begin Source File

SOURCE=.\mnu.c
# End Source File
# Begin Source File

SOURCE=.\msg.c
# End Source File
# Begin Source File

SOURCE=.\msged.c
# End Source File
# Begin Source File

SOURCE=.\mxbt.c
# End Source File
# Begin Source File

SOURCE=.\normalc.c
# End Source File
# Begin Source File

SOURCE=.\nshow.c
# End Source File
# Begin Source File

SOURCE=.\quick.c
# End Source File
# Begin Source File

SOURCE=.\quote.c
# End Source File
# Begin Source File

SOURCE=.\readmail.c
# End Source File
# Begin Source File

SOURCE=.\screen.c
# End Source File
# Begin Source File

SOURCE=.\strextra.c
# End Source File
# Begin Source File

SOURCE=.\system.c
# End Source File
# Begin Source File

SOURCE=.\template.c
# End Source File
# Begin Source File

SOURCE=.\textfile.c
# End Source File
# Begin Source File

SOURCE=.\timezone.c
# End Source File
# Begin Source File

SOURCE=.\userlist.c
# End Source File
# Begin Source File

SOURCE=.\vsev.c
# End Source File
# Begin Source File

SOURCE=.\vsevops.c
# End Source File
# Begin Source File

SOURCE=.\win.c
# End Source File
# Begin Source File

SOURCE=.\winntscr.c
# End Source File
# Begin Source File

SOURCE=.\wrap.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\winsys.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
