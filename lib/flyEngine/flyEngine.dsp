# Microsoft Developer Studio Project File - Name="Fly3D_engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Fly3D_engine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "flyEngine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "flyEngine.mak" CFG="Fly3D_engine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Fly3D_engine - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Fly3D_engine - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Fly3D_engine - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "FLY_ENGINE_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x416 /d "NDEBUG"
# ADD RSC /l 0x416 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 opengl32.lib glu32.lib winmm.lib user32.lib advapi32.lib dxguid.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"LIBC.LIB" /out:"../../flyEngine.dll"
# SUBTRACT LINK32 /pdb:none /debug
# Begin Special Build Tool
OutDir=.\Release
TargetName=flyEngine
SOURCE="$(InputPath)"
PostBuild_Cmds=if exist "$(OutDir)\$(TargetName).lib" copy "$(OutDir)\$(TargetName).lib" "..\$(TargetName).lib"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Fly3D_engine - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "\mssdk\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "FLY_ENGINE_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x416 /d "_DEBUG"
# ADD RSC /l 0x416 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib glu32.lib winmm.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"LIBC.lib" /out:"../../flyEngine.dll" /pdbtype:sept /libpath:"\mssdk\lib"
# Begin Special Build Tool
OutDir=.\Debug
TargetName=flyEngine
SOURCE="$(InputPath)"
PostBuild_Cmds=if exist "$(OutDir)\$(TargetName).lib" copy "$(OutDir)\$(TargetName).lib" "..\$(TargetName).lib"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Fly3D_engine - Win32 Release"
# Name "Fly3D_engine - Win32 Debug"
# Begin Group "cpp"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\flyAI.cpp
# End Source File
# Begin Source File

SOURCE=.\flyAnimatedMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\flyBaseObject.cpp
# End Source File
# Begin Source File

SOURCE=.\flyBezier.cpp
# End Source File
# Begin Source File

SOURCE=.\flyBoundBox.cpp
# End Source File
# Begin Source File

SOURCE=.\flyBspNode.cpp
# End Source File
# Begin Source File

SOURCE=.\flyBspObject.cpp
# End Source File
# Begin Source File

SOURCE=.\flyConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\flyDLL.cpp
# End Source File
# Begin Source File

SOURCE=.\flyEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\flyEngineBspRecurse.cpp
# End Source File
# Begin Source File

SOURCE=.\flyEngineDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\flyEngineGetSet.cpp
# End Source File
# Begin Source File

SOURCE=.\flyEngineInit.cpp
# End Source File
# Begin Source File

SOURCE=.\flyEngineLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\flyFace.cpp
# End Source File
# Begin Source File

SOURCE=.\flyFile.cpp
# End Source File
# Begin Source File

SOURCE=.\flyFrustum.cpp
# End Source File
# Begin Source File

SOURCE=.\flyHashList.cpp
# End Source File
# Begin Source File

SOURCE=.\flyInputMap.cpp
# End Source File
# Begin Source File

SOURCE=.\flyLight.cpp
# End Source File
# Begin Source File

SOURCE=.\flyLocalSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\flyMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\flyMultiplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\flyOcTree.cpp
# End Source File
# Begin Source File

SOURCE=.\flyParamDesc.cpp
# End Source File
# Begin Source File

SOURCE=.\flyParticle.cpp
# End Source File
# Begin Source File

SOURCE=.\flyPolygon.cpp
# End Source File
# Begin Source File

SOURCE=.\flyShader.cpp
# End Source File
# Begin Source File

SOURCE=.\flySkeletonMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\flySound.cpp
# End Source File
# Begin Source File

SOURCE=.\flyStaticMesh.cpp
# End Source File
# Begin Source File

SOURCE=.\flyString.cpp
# End Source File
# End Group
# Begin Group "h"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\flyAI.h
# End Source File
# Begin Source File

SOURCE=..\flyAnimatedMesh.h
# End Source File
# Begin Source File

SOURCE=..\flyArray.h
# End Source File
# Begin Source File

SOURCE=..\flyBaseObject.h
# End Source File
# Begin Source File

SOURCE=..\flyBezier.h
# End Source File
# Begin Source File

SOURCE=..\flyBoundBox.h
# End Source File
# Begin Source File

SOURCE=..\flyBspNode.h
# End Source File
# Begin Source File

SOURCE=..\flyBspObject.h
# End Source File
# Begin Source File

SOURCE=..\flyConsole.h
# End Source File
# Begin Source File

SOURCE=..\flyDefine.h
# End Source File
# Begin Source File

SOURCE=..\flyDesc.h
# End Source File
# Begin Source File

SOURCE=..\flyDll.h
# End Source File
# Begin Source File

SOURCE=..\flyEngine.h
# End Source File
# Begin Source File

SOURCE=..\flyFace.h
# End Source File
# Begin Source File

SOURCE=..\flyFile.h
# End Source File
# Begin Source File

SOURCE=..\flyFrustum.h
# End Source File
# Begin Source File

SOURCE=..\flyHashList.h
# End Source File
# Begin Source File

SOURCE=..\flyInputMap.h
# End Source File
# Begin Source File

SOURCE=..\flyLight.h
# End Source File
# Begin Source File

SOURCE=..\flyLocalSystem.h
# End Source File
# Begin Source File

SOURCE=..\flyMesh.h
# End Source File
# Begin Source File

SOURCE=..\flyOcTree.h
# End Source File
# Begin Source File

SOURCE=..\flyParticle.h
# End Source File
# Begin Source File

SOURCE=..\flyPolygon.h
# End Source File
# Begin Source File

SOURCE=..\flyShader.h
# End Source File
# Begin Source File

SOURCE=..\flySkeletonMesh.h
# End Source File
# Begin Source File

SOURCE=..\flySound.h
# End Source File
# Begin Source File

SOURCE=..\flyStaticMesh.h
# End Source File
# Begin Source File

SOURCE=..\flyString.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\cpuid.h
# End Source File
# Begin Source File

SOURCE=..\fly3D.h
# End Source File
# Begin Source File

SOURCE=.\flyEngine.rc
# End Source File
# Begin Source File

SOURCE=.\cpuid.lib
# End Source File
# Begin Source File

SOURCE=.\unzip_zlib.lib
# End Source File
# End Target
# End Project
