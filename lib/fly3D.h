/*! \file */

#ifdef FINDMEMLEAK
	#ifdef _DEBUG
		#include <afxwin.h>
		#define new DEBUG_NEW
	#else
		#include <windows.h>
	#endif
#else
	#include <windows.h>
#endif

#include <mmsystem.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include "wglATI.h"
#include "glATI.h"
#include "glExtension.h"

#include "unzip.h"

#include "flyDefine.h"
#include "flyMath.h"
#include "flyDirectX.h"
#include "flyRender.h"

#ifdef FLY_ENGINE_H
#else
#define FLY_ENGINE_H

#ifdef FLY_ENGINE_EXPORTS
#define FLY_ENGINE_API __declspec(dllexport)
#else
#define FLY_ENGINE_API __declspec(dllimport)
#endif

class FLY_ENGINE_API flyString;
class FLY_ENGINE_API flyBoundBox;
class FLY_ENGINE_API flyFrustum;
class FLY_ENGINE_API flyLocalSystem;
class FLY_ENGINE_API flyBaseObject;
class FLY_ENGINE_API flySound;
class FLY_ENGINE_API flyPolygon;
class FLY_ENGINE_API flyFace;
class FLY_ENGINE_API flyMesh;
class FLY_ENGINE_API flyAnimatedMesh;
class FLY_ENGINE_API flySkeletonMesh;
class FLY_ENGINE_API flyBezierCurve;
class FLY_ENGINE_API flyBezierPatch;
class FLY_ENGINE_API flyParticle;
class FLY_ENGINE_API flyOcTree;
class FLY_ENGINE_API flyOcTreeNode;
class FLY_ENGINE_API flyBspNode;
class FLY_ENGINE_API flyBspObject;
class FLY_ENGINE_API flyStaticMesh;
class FLY_ENGINE_API flyLightMap;
class FLY_ENGINE_API flyLightMapPic;
class FLY_ENGINE_API flyLightVertex;
class FLY_ENGINE_API flyClassDesc;
class FLY_ENGINE_API flyParamDesc;
class FLY_ENGINE_API flyConsole;
class FLY_ENGINE_API flyShader;
class FLY_ENGINE_API flyShaderFunc;
class FLY_ENGINE_API flyShaderPass;
class FLY_ENGINE_API flyInputMap;
class FLY_ENGINE_API flyFile;
class FLY_ENGINE_API flyDll;
class FLY_ENGINE_API flyDllGroup;
class FLY_ENGINE_API flyHashList;
class FLY_ENGINE_API flyEngine;

#include "flyArray.h"
#include "flyString.h"
#include "flyBoundBox.h"
#include "flyFrustum.h"
#include "flyLocalSystem.h"
#include "flyParticle.h"
#include "flyBaseObject.h"
#include "flyBspObject.h"
#include "flyBspNode.h"
#include "flyOcTree.h"
#include "flyBezier.h"
#include "flyPolygon.h"
#include "flyFace.h"
#include "flyMesh.h"
#include "flyAnimatedMesh.h"
#include "flySkeletonMesh.h"
#include "flyStaticMesh.h"
#include "flyLight.h"
#include "flyDesc.h"
#include "flyConsole.h"
#include "flySound.h"
#include "flyShader.h"
#include "flyInputMap.h"
#include "flyFile.h"
#include "flyDll.h"
#include "flyAI.h"
#include "flyHashList.h"
#include "flyEngine.h"

//! Window handle used by the engine
extern FLY_ENGINE_API HWND g_flyhwnd;
//! Instance handle used by the engine
extern FLY_ENGINE_API HINSTANCE g_flyhinst;
//! global flyEngine instance
extern FLY_ENGINE_API flyEngine *g_flyengine;
//! Fly3D GUID
extern FLY_ENGINE_API GUID g_flyGUID;
//! Flags indicating processor features available like MMX,3DNow,SSE,SS2
extern FLY_ENGINE_API int g_processor_features;

//! global engine initialisation method
FLY_ENGINE_API void fly_init_engine(HWND hWnd,HINSTANCE hInst,int appid=FLY_APPID_NONE);
//! global engine initialisation method from an alredy initialized pointer (used when subclassing flyEngine)
FLY_ENGINE_API void fly_set_engine(flyEngine *eng,HWND hWnd,HINSTANCE hInst,int appid=FLY_APPID_NONE);
//! global engine release method
FLY_ENGINE_API void fly_free_engine();

//! Read data from the Fly3D section in windows registry 
FLY_ENGINE_API void fly_get_profile(LPTSTR section, LPTSTR key, LPBYTE lpData, DWORD cbData);
//! Write data to the Fly3D section in windows registry 
FLY_ENGINE_API void fly_write_profile(LPTSTR section, LPCTSTR key, DWORD dwType, CONST BYTE *lpData, DWORD cbData);

#endif
