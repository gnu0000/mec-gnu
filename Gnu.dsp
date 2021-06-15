# Microsoft Developer Studio Project File - Name="Gnu" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Gnu - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Gnu.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Gnu.mak" CFG="Gnu - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Gnu - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Gnu - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Gnu - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
OutDir=.\Release
ProjDir=.
SOURCE="$(InputPath)"
PostBuild_Cmds=if  not  defined  ITIINC  echo  You  need  to  set  the  ITIINC  environment  variable!  	if  not  defined  ITILIB  echo  You  need  to  set  the  ITILIB  environment  variable!  	if  defined  ITIINC  copy  $(ProjDir)\Config\G_Config.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Dynaview\G_ColoredView.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Dynaview\G_DynaView.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Expr\G_Expr.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Gzfile\G_GZFile.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Gzfile\G_SecureZipArchive.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Gzfile\zlib.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Http\G_Http.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\PKC\G_PKC.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Rc4\G_Rc4.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\String\G_csv.h  %ITIINC%\  	if  defined  ITILIB  copy  $(OutDir)\*.lib  %ITILIB%\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Gnu - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AS_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\Gnu_Debug.lib"
# Begin Special Build Tool
OutDir=.\Debug
ProjDir=.
SOURCE="$(InputPath)"
PostBuild_Cmds=if  not  defined  ITIINC  echo  You  need  to  set  the  ITIINC  environment  variable!  	if  not  defined  ITILIB  echo  You  need  to  set  the  ITILIB  environment  variable!  	if  defined  ITIINC  copy  $(ProjDir)\Config\G_Config.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Dynaview\G_ColoredView.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Dynaview\G_DynaView.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Expr\G_Expr.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Gzfile\G_GZFile.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Gzfile\G_SecureZipArchive.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Gzfile\zlib.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Http\G_Http.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\PKC\G_PKC.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\Rc4\G_Rc4.h  %ITIINC%\  	if  defined  ITIINC  copy  $(ProjDir)\String\G_csv.h  %ITIINC%\  	if  defined  ITILIB  copy  $(OutDir)\*.lib  %ITILIB%\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Gnu - Win32 Release"
# Name "Gnu - Win32 Debug"
# Begin Group "Dynaview Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DynaView\ColoredView.cpp
# End Source File
# Begin Source File

SOURCE=.\DynaView\DynaView.cpp
# End Source File
# Begin Source File

SOURCE=.\DynaView\G_ColoredView.h
# End Source File
# Begin Source File

SOURCE=.\DynaView\G_DynaView.h
# End Source File
# End Group
# Begin Group "GZFile Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\GZFile\G_GZFile.h
# End Source File
# Begin Source File

SOURCE=.\GZFile\G_SecureZipArchive.h
# End Source File
# Begin Source File

SOURCE=.\GZFile\GZFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GZFile\gzrc4.C
# End Source File
# Begin Source File

SOURCE=.\GZFile\gzrc4.h
# End Source File
# Begin Source File

SOURCE=.\GZFile\SecureZipArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\GZFile\unzip.c
# End Source File
# Begin Source File

SOURCE=.\GZFile\unzip.h
# End Source File
# Begin Source File

SOURCE=.\GZFile\zip.c
# End Source File
# Begin Source File

SOURCE=.\GZFile\zip.h
# End Source File
# End Group
# Begin Group "Common Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "String Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\string\CSV.cpp
# End Source File
# Begin Source File

SOURCE=.\string\G_CSV.h
# End Source File
# End Group
# Begin Group "Expr Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Expr\Expr.cpp
# End Source File
# Begin Source File

SOURCE=.\Expr\Expr_priv.h
# End Source File
# Begin Source File

SOURCE=.\Expr\G_Expr.h
# End Source File
# End Group
# Begin Group "RC4 Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Rc4\G_Rc4.h
# End Source File
# Begin Source File

SOURCE=.\Rc4\rc4.cpp
# End Source File
# End Group
# Begin Group "Configuration"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Config\G_Config.h
# End Source File
# Begin Source File

SOURCE=.\Config\Registry.cpp
# End Source File
# End Group
# Begin Group "PKC Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PKC\G_PKC.h
# End Source File
# Begin Source File

SOURCE=.\PKC\PKC.cpp
# End Source File
# End Group
# Begin Group "Http Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Http\G_Http.h
# End Source File
# Begin Source File

SOURCE=.\Http\http.cpp
# End Source File
# End Group
# End Target
# End Project
