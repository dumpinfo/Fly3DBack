/*! \file */

#ifdef FLY_RENDER_H
#else
#define FLY_RENDER_H

#ifdef FLY_RENDER_EXPORTS
#define FLY_RENDER_API __declspec(dllexport)
#else
#define FLY_RENDER_API __declspec(dllimport)
#endif

//! Number of letters per line in font texture
#define FLY_FONTS_NUM			16

//! Number of letters per line in font texture
#define FLY_FONTS_FACTOR		0.0625f

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

#include <stdio.h>
#include <vfw.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include "wglATI.h"
#include "glATI.h"
#include "glExtension.h"

#include "flyPicture.h"
#include "flyVideo.h"

//! Render management class
/*!
	This class implements the engine's rendering module and is an interface with the 
	graphics API, OpenGL. It contains members related to the window where rendering 
	will be performed, like the window and device context handlers. It also has members
	that store the rendering modes and options, like an array of available video modes, 
	the color depth, the screen resolution, and many flags that signalize for turning on 
	anti-aliasing, activating the full-screen mode, enabling the use of a stencil buffer, 
	and so on. Among the various methods present in the class are those for initialising 
	the render entities, changing to full-screen mode, resizing the render window, 
	drawing text, and so on.
*/
class FLY_RENDER_API flyRender
{
public:

	HWND m_hwnd;		//!< window handle
	HINSTANCE m_hinst;	//!< instance handle
	HGLRC m_hrc;		//!< OpenGL handle
	HDC m_hdc;			//!< device context handle
	RECT m_winrect;		//!< window rectangle

	int m_numvideomodes;		//!< number of available video modes
	DEVMODE m_videomodes[1024];	//!< array of video modes

	int m_numrendermodes;						//!< number of rendering modes
	PIXELFORMATDESCRIPTOR m_rendermodes[1024];	//!< array of rendering modes
	int m_rendermodesindex[1024];				//!< index of rendering modes

	int m_colorbits;		//!< number of color bits
	int m_stencilbits;		//!< number of stencil buffer bits (per pixel)
	int m_depthbits;		//!< number of depth buffer bits (per pixel)
	int m_accumbits;		//!< number of accumulation buffer bits
	int m_alphabits;		//!< number of alpha buffer bits
	int m_numtextureunits;	//!< number of texture units
	int m_numhwlights;		//!< number of hardware lights
	int m_cursorx,			//!< x position of the last written letter
		m_cursory;			//!< y position of the last written letter
	int m_curtime;			//!< render current time

	static int s_fontspic;			//!< fonts texture index
	static int s_fontspicsize;		//!< fonts texture size in pixels
	static int s_fontspiccharsize;	//!< fonts char size in pixels
	static int s_consolepic;		//!< console texture index
	static int s_screensizex;		//!< screen width in pixels
	static int s_screensizey;		//!< screen height in pixels
	static int s_fullscreen;		//!< fullscreen flag
	static int s_selvideomode;		//!< index of selected video mode
	static int s_selrendermode;		//!< index of selected render mode
	static int s_clearbg;			//!< background clear flag
	static int s_antialias;			//!< anti-aliasing flag
	static int s_wireframe;			//!< wireframe flag
	static int s_fog;				//!< fog flag
	static int s_stencil;			//!< stencil flag
	static int s_fontswidth[256];	//!< width of each letter of the font
	static int s_texdroplevel;		//!< texture level of detail (0 no drop, 1 drop half size, ...)
	static int s_videores;			//!< resolution for video files (0-novideo, 1-2x2, 2-4x4, ..., 8-256x256, ...)

	static float s_brightness;		//!< brightness level
	static float s_ambient;			//!< ambient lighting level
	static float s_aspect;			//!< screen aspect ratio
	static float s_camangle;		//!< camera angle
	static float s_nearplane;		//!< near rendering plane
	static float s_farplane;		//!< far rendering plane
	static float s_background[4];	//!< background color

	//! Default constructor, gets the window and instance handlers
	flyRender(HWND hWnd,HINSTANCE hInst) 
	{ 
		m_hwnd=hWnd; 
		m_hinst=hInst; 
		m_hdc=0; 
		m_hrc=0; 
		m_numvideomodes=0;
		m_numrendermodes=0;
		m_numtextureunits=1;
		m_numhwlights=4;
		m_colorbits=0;
		m_stencilbits=0;
		m_depthbits=0;
		m_accumbits=0;
		m_alphabits=0;
		m_curtime=0;
	};

	//! Default destructor
	virtual ~flyRender()	
	{ destroy(); };

	//! Create the rendering manager
	int create();
	//! Destroy the rendering manager
	void destroy();
	//! Initialise the rendering manager
	void init();
	//! Resize the rendering window
	void resize(int sx,int sy);
	//! Change to fullscreen mode
	void set_full_screen();

	//! Enter drawing mode
	void begin_draw();
	//! Leave drawing mode
	void end_draw();

	//! Enter 2D drawing mode
	void begin_draw2d();
	//! Leave 2D drawing mode
	void end_draw2d();

	//! Draw text aligned to the left of the given position
	void draw_text(int x,int y,const char *text,int size=16,int n=-1);
	//! Draw text centralized on the given position
	void draw_text_center(int x,int y,const char *text,int size=16);
	//! Calculates the horizontal size of a text string
	int get_text_size(const char *text, int size);
	//! draws progress info (used on loading)
	void draw_progress(const char *text, int progress);
};

//! global flyRender instance
extern FLY_RENDER_API flyRender *g_flyrender;

#include "flyTexCache.h"

//! global render manager initialisation method
FLY_RENDER_API void fly_init_render(HWND hWnd,HINSTANCE hInst);
//! global render manager release method
FLY_RENDER_API void fly_free_render();

#endif