# Microsoft Developer Studio Project File - Name="FLHook" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=FLHook - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FLHook.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FLHook.mak" CFG="FLHook - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FLHook - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "FLHook - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FLHook - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FLHOOK_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FLHOOK_EXPORTS" /YX /FD /EHa /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 winmm.lib psapi.lib wsock32.lib ws2_32.lib ../source/libs/server.lib ../source/libs/common.lib ../source/libs/remoteclient.lib ../source/libs/dalib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "FLHook - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FLHOOK_EXPORTS" /YX /FD /ZI /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zd /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FLHOOK_EXPORTS" /FR /YX /FD /ZI /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib psapi.lib wsock32.lib ws2_32.lib ../source/libs/server.lib ../source/libs/common.lib ../source/libs/remoteclient.lib ../source/libs/dalib.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "FLHook - Win32 Release"
# Name "FLHook - Win32 Debug"
# Begin Group "AdminCommands"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\CCmds.cpp
# End Source File
# Begin Source File

SOURCE=..\source\CCmds.h
# End Source File
# Begin Source File

SOURCE=..\source\CConsole.cpp
# End Source File
# Begin Source File

SOURCE=..\source\CConsole.h
# End Source File
# Begin Source File

SOURCE=..\source\CInGame.cpp
# End Source File
# Begin Source File

SOURCE=..\source\CInGame.h
# End Source File
# Begin Source File

SOURCE=..\source\CSocket.cpp
# End Source File
# Begin Source File

SOURCE=..\source\CSocket.h
# End Source File
# End Group
# Begin Group "flcodec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\flcodec.cpp
# End Source File
# Begin Source File

SOURCE=..\source\flcodec.h
# End Source File
# End Group
# Begin Group "Hook"

# PROP Default_Filter ""
# Begin Group "dll imports"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\common.h
# End Source File
# Begin Source File

SOURCE=..\source\dalib.h
# End Source File
# Begin Source File

SOURCE=..\source\remoteclient.h
# End Source File
# Begin Source File

SOURCE=..\source\server.h
# End Source File
# End Group
# Begin Group "callbacks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\HkCbChat.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkCbDamage.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkCbDeath.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkCbDisconnect.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkCbIServerImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkCbOther.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkTimers.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkUserCmd.cpp
# End Source File
# End Group
# Begin Group "functions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\HkFuncLog.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkFuncMsg.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkFuncOther.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkFuncPlayers.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkFuncTools.cpp
# End Source File
# End Group
# Begin Group "data"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\HkDataBaseMarket.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkDataBaseOther.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\source\HkError.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkFLIni.cpp
# End Source File
# Begin Source File

SOURCE=..\source\HkInit.cpp
# End Source File
# Begin Source File

SOURCE=..\source\Hook.h
# End Source File
# End Group
# Begin Group "xtrace"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\xtrace.cpp

!IF  "$(CFG)" == "FLHook - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FLHook - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\source\xtrace.h

!IF  "$(CFG)" == "FLHook - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "FLHook - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "wildcards"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\wildcards.cpp
# End Source File
# Begin Source File

SOURCE=..\source\wildcards.hh
# End Source File
# End Group
# Begin Group "binary tree"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\binarytree.cpp
# End Source File
# Begin Source File

SOURCE=..\source\binarytree.h
# End Source File
# End Group
# Begin Group "blowfish"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\source\blowfish.cpp
# End Source File
# Begin Source File

SOURCE=..\source\blowfish.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\source\exceptioninfo.h
# End Source File
# Begin Source File

SOURCE=..\source\FLHook.cpp
# End Source File
# Begin Source File

SOURCE=..\source\global.h
# End Source File
# Begin Source File

SOURCE=..\source\Settings.cpp
# End Source File
# Begin Source File

SOURCE=..\source\Tools.cpp
# End Source File
# Begin Source File

SOURCE=..\source\Version.rc
# End Source File
# End Target
# End Project
