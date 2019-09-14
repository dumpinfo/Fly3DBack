#define GLEXT_PROTOTYPE
#include "../flyDefine.h"
#include "../flyRender.h"

FLY_RENDER_API flyRender *g_flyrender=0;

int flyRender::s_fontspic=-1;
int flyRender::s_fontspicsize=256;
int flyRender::s_fontspiccharsize=16;
int flyRender::s_consolepic=-1;
int flyRender::s_screensizex=0;
int flyRender::s_screensizey=0;
int flyRender::s_fullscreen=0;
int flyRender::s_selvideomode=0;
int flyRender::s_selrendermode=0;
int flyRender::s_clearbg=1;
int flyRender::s_antialias=0;
int flyRender::s_wireframe=0;
int flyRender::s_fog=0;
int flyRender::s_stencil=0;
int flyRender::s_fontswidth[256];
int flyRender::s_texdroplevel=0;
int flyRender::s_videores=7;

float flyRender::s_brightness=1.8f;
float flyRender::s_ambient=0.2f;
float flyRender::s_aspect=1.0f;
float flyRender::s_camangle=60;
float flyRender::s_nearplane=10;
float flyRender::s_farplane=5000;
float flyRender::s_background[4]={ 0,0,0,1 };

void APIENTRY null_glActiveTextureARB(GLenum target)
{ ; };
void APIENTRY null_glClientActiveTextureARB(GLenum target)
{ ; };

void fly_get_profile(LPTSTR section, LPTSTR key, LPBYTE lpData, DWORD cbData)
{
	HKEY key1,key2,key3,key4;

	if (ERROR_SUCCESS==RegOpenKeyEx(HKEY_CURRENT_USER,"Software",0,KEY_READ,&key1))
	{
		if (ERROR_SUCCESS==RegOpenKeyEx(key1,FLY_REGISTRYKEY_PRIMARY,0,KEY_READ,&key2))
		{
			if (ERROR_SUCCESS==RegOpenKeyEx(key2,FLY_REGISTRYKEY_SECONDARY,0,KEY_READ,&key3))
			{
				if (ERROR_SUCCESS==RegOpenKeyEx(key3,section,0,KEY_READ,&key4))
				{
					RegQueryValueEx(key4, key, 0, 0, lpData, &cbData);
					RegCloseKey(key4);
				}
				RegCloseKey(key3);
			}
			RegCloseKey(key2);
		}
		RegCloseKey(key1);
	}
}

FLY_RENDER_API void fly_init_render(HWND hWnd,HINSTANCE hInst)
{
	fly_free_render();
	
	fly_get_profile("Settings","lmscale", (unsigned char *)&flyRender::s_brightness, sizeof(float));
	fly_get_profile("Settings","lmbias", (unsigned char *)&flyRender::s_ambient, sizeof(float));
	fly_get_profile("Settings","videomode", (unsigned char *)&flyRender::s_selvideomode, sizeof(int));
	fly_get_profile("Settings","rendermode", (unsigned char *)&flyRender::s_selrendermode, sizeof(int));

	if (hWnd&&hInst)
	{
	g_flyrender=new flyRender(hWnd,hInst);
	g_flyrender->create();
	g_flyrender->init();
	}
	g_flytexcache=new flyTexCache();
}

FLY_RENDER_API void fly_free_render()
{
	delete g_flytexcache;
	g_flytexcache=0;

	if (g_flyrender==0)
		return;

	if (flyRender::s_fullscreen)
		ChangeDisplaySettings(NULL,CDS_FULLSCREEN);

	g_flyrender->destroy();
	delete g_flyrender;
	g_flyrender=0;
}

int flyRender::create()
{
	int i,j;
	
	m_hdc=GetDC(m_hwnd);
	m_colorbits=GetDeviceCaps(m_hdc,BITSPIXEL);

	int color;
	fly_get_profile("Settings","colorbits", (unsigned char *)&color, sizeof(int));
	if (m_colorbits!=color)
	{
		MessageBox(m_hwnd,"Fly3D flyRender configuration not set or display color depth has been changed.\nShould run flyConfig.exe to select a new flyRender mode.","Fly3D",MB_OK|MB_ICONINFORMATION);
		s_selrendermode=0;
	}

	i=0;
	m_numvideomodes=0;
	while (EnumDisplaySettings(NULL,i,&m_videomodes[m_numvideomodes]))
	{
		if ((int)m_videomodes[m_numvideomodes].dmBitsPerPel==m_colorbits)
			m_numvideomodes++;
		i++;
	}

	i=1;
	j=DescribePixelFormat(m_hdc,i,sizeof(PIXELFORMATDESCRIPTOR),&m_rendermodes[i]);
	m_numrendermodes=0;
	while(i<j)
	{
		DescribePixelFormat(m_hdc,i,sizeof(PIXELFORMATDESCRIPTOR),&m_rendermodes[m_numrendermodes]);
		if (m_rendermodes[m_numrendermodes].iPixelType==0 && 
			m_rendermodes[m_numrendermodes].cColorBits==m_colorbits &&
			(m_rendermodes[m_numrendermodes].dwFlags&PFD_DOUBLEBUFFER))
			m_rendermodesindex[m_numrendermodes++]=i;
		i++;
	}

	if (s_selvideomode>m_numvideomodes)
		s_selvideomode=0;
	if (s_selrendermode>m_numrendermodes)
		s_selrendermode=0;

	if (SetPixelFormat(m_hdc, m_rendermodesindex[s_selrendermode], &m_rendermodes[s_selrendermode]) == FALSE) 
		return FALSE;

	m_hrc = wglCreateContext( m_hdc );
	wglMakeCurrent( m_hdc, m_hrc );

	RECT rect;
	GetClientRect(m_hwnd,&rect);
	resize(rect.right,rect.bottom);

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glPixelStorei(GL_PACK_ALIGNMENT,1);

	glHint(GL_FOG_HINT,GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1,-1);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
	float f[4]={ 0.3f,0.3f,0.3f,1 };
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,f);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,f);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,8);

	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glGetIntegerv(GL_MAX_LIGHTS,&m_numhwlights);
	if (m_numhwlights>FLY_MAX_HWLIGHTS)
		m_numhwlights=FLY_MAX_HWLIGHTS;
	
	glGetIntegerv(GL_DEPTH_BITS,&m_depthbits);
	glGetIntegerv(GL_STENCIL_BITS,&m_stencilbits);
	
	glGetIntegerv(GL_ALPHA_BITS,&m_alphabits);
	glGetIntegerv(GL_ACCUM_ALPHA_BITS,&i);
	m_accumbits=i;
	glGetIntegerv(GL_ACCUM_RED_BITS,&i);
	m_accumbits+=i;
	glGetIntegerv(GL_ACCUM_BLUE_BITS,&i);
	m_accumbits+=i;
	glGetIntegerv(GL_ACCUM_GREEN_BITS,&i);
	m_accumbits+=i;

	glColor4ub(255,255,255,255);

	InitGLextensions();

	if (ARB_multitexture==0)
		m_numtextureunits=1;
	else
	{
		glGetIntegerv(GL_MAX_ACTIVE_TEXTURES_ARB,&m_numtextureunits);
		m_numtextureunits=2;
		for( int i=1;i<m_numtextureunits;i++ )
			{
			glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisable(GL_TEXTURE_2D);
			}
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
	}

	if (NV_vertex_program)
	{
		glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);
		glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 4, GL_MODELVIEW, GL_IDENTITY_NV);
		glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 8, GL_MODELVIEW, GL_INVERSE_TRANSPOSE_NV);
		glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 12, GL_MODELVIEW, GL_INVERSE_NV);

		glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 20, 0.0f, 0.5f, 1.0f, 2.0f);

		#define PI 3.14159265359f
		glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 21, PI, 0.5f, 2.0f*PI, 1.0f/(2.0f*PI));
		glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 22, 1.0f, -0.5f, 1.0f/24.0f,  -1.0f/720.0f);
		glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 23, 1.0f, -1.0f/6.0f, 1.0f/120.0f, -1.0f/5040.0f);
	}

	return 1;
}

void flyRender::destroy()
{
	if (m_hrc==0)
		return;

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(m_hrc);
	m_hrc=0;

	ReleaseDC(m_hwnd,m_hdc);
	m_hdc=0;
}

void flyRender::resize(int sx,int sy)
{
	if (m_hrc==0)
		return;
    glViewport(0, 0, sx, sy);
	if (sy==0)
		s_aspect=1;
	else s_aspect=(float)sx/sy;
	s_screensizex=sx;
	s_screensizey=sy;
}

void flyRender::init()
{
	glClearColor( s_background[0],s_background[1], s_background[2], 1 );
    glClearDepth( 1 );

	glLineWidth(1);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glShadeModel(GL_SMOOTH);
	glPolygonMode(GL_FRONT,s_wireframe?GL_LINE:GL_FILL);
	glPolygonMode(GL_BACK,s_wireframe?GL_LINE:GL_FILL);
	glPointSize(1.0f);

	if (s_fog)
		{
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE,GL_LINEAR);
		glFogf(GL_FOG_START,s_farplane*0.6f);
		glFogf(GL_FOG_END,s_farplane);
		glFogfv(GL_FOG_COLOR,s_background );
		}
	else glDisable(GL_FOG);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

void flyRender::begin_draw()
{
	if (m_hrc==0)
		return;
	int clear=GL_DEPTH_BUFFER_BIT;
	if (s_clearbg)
		clear|=GL_COLOR_BUFFER_BIT;
	if (s_stencil)
		clear|=GL_STENCIL_BUFFER_BIT;
	init();
	glClear( clear );
}

void flyRender::end_draw()
{
	SwapBuffers(m_hdc);
}

void flyRender::set_full_screen()
{
	if (m_videomodes && s_selvideomode<m_numvideomodes)
	{
	if (s_fullscreen)
		{
		GetWindowRect(m_hwnd,&m_winrect);
		if (DISP_CHANGE_SUCCESSFUL==ChangeDisplaySettings(&m_videomodes[s_selvideomode],CDS_FULLSCREEN))
			{
			s_screensizex=m_videomodes[s_selvideomode].dmPelsWidth;
			s_screensizey=m_videomodes[s_selvideomode].dmPelsHeight;
			SetWindowLong(m_hwnd,GWL_STYLE,WS_POPUP|WS_CLIPSIBLINGS|WS_CLIPCHILDREN);
			ShowWindow(m_hwnd,SW_SHOWMAXIMIZED);
			}
		else s_fullscreen=0;
		}
	else
		{
		ChangeDisplaySettings(NULL,CDS_FULLSCREEN);
		SetWindowLong(m_hwnd,GWL_STYLE,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
		ShowWindow(m_hwnd,SW_RESTORE);
		SetWindowPos(m_hwnd,HWND_TOP,m_winrect.left,m_winrect.top,m_winrect.right-m_winrect.left,m_winrect.bottom-m_winrect.top,SWP_SHOWWINDOW);
		}
	}
}

void flyRender::begin_draw2d()
{
	// sets 2d camera in screen coordinates
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,s_screensizex,0,s_screensizey,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// sets standard text state
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_FOG);
	glColor4ub(255,255,255,255);
}

void flyRender::end_draw2d()
{
	// restore text state
	glColor4ub(255,255,255,255);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	if (s_fog) glEnable(GL_FOG);
}

void flyRender::draw_text(int x,int y,const char *text,int size,int n)
{
	const unsigned char *utext=(unsigned char *)text;

	if (flyRender::s_fontspic==-1)
		return;

	int len=(n==-1?strlen(text):n);

	g_flytexcache->sel_tex(flyRender::s_fontspic);
	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	g_flyrender->m_cursorx=x;
	g_flyrender->m_cursory=y;
	y=flyRender::s_screensizey-y-size;

	glBegin(GL_QUADS);

	float xx,yy;
	for( int i=0;i<len;i++ )
		if(utext[i]=='\n')
		{
			g_flyrender->m_cursory+=size;
			y-=size;
			x=g_flyrender->m_cursorx;
		}
		else
		{
			xx=(utext[i]%FLY_FONTS_NUM)*FLY_FONTS_FACTOR;
			yy=(utext[i]/FLY_FONTS_NUM)*FLY_FONTS_FACTOR;

			glTexCoord2f(xx+FLY_FONTS_FACTOR,yy+FLY_FONTS_FACTOR);
			glVertex2i(x+size,y);

			glTexCoord2f(xx+FLY_FONTS_FACTOR,yy);
			glVertex2i(x+size,y+size);

			glTexCoord2f(xx,yy);
			glVertex2i(x,y+size);

			glTexCoord2f(xx,yy+FLY_FONTS_FACTOR);
			glVertex2i(x,y);

			x+=flyRender::s_fontswidth[utext[i]]*size/flyRender::s_fontspiccharsize;
		}

	glEnd();

	g_flyrender->m_cursorx=x;
}

void flyRender::draw_text_center(int x,int y,const char *text,int size)
{
	const unsigned char *utext=(unsigned char *)text;
	int s=0;

	unsigned char* c=(unsigned char*)strchr(text,'\n');
	if(c==0)
	{
		s=get_text_size(text,size);
		draw_text(x-(s/2),y,text,size);
	}
	else
	{
		const unsigned char *t=utext;
		for( ;utext!=c;utext++ )
			s+=flyRender::s_fontswidth[*utext]*size/flyRender::s_fontspiccharsize;

		draw_text(x-(s/2),y,(char *)t,size,utext-t);

		draw_text_center(x,y+size,(char *)++utext,size);
	}
}

int flyRender::get_text_size(const char *text, int size)
{
	const unsigned char *utext=(unsigned char *)text;
	int ret=0,i=0,j=strlen(text);
	for( ;i<j;i++ )
		ret+=flyRender::s_fontswidth[utext[i]];
	return ret*size/flyRender::s_fontspiccharsize;
}

void flyRender::draw_progress(const char *text, int progress)
{
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	begin_draw2d();

	g_flytexcache->clear_tex_state();

	int fontsize=16;
	int x=0,y=s_screensizey/2;
	int sx=s_screensizex,sy=fontsize+4;
	int i=sx*progress/100;
	unsigned char 
		color_back1[4]={ 24,64,128,255 },
		color_back2[4]={ 24,96,128,255 },
		color_text[4]={ 200,200,200,255 };
	
	glColor4ubv(color_back2);
	glBegin(GL_QUADS);
	glVertex2i(x,y);
	glVertex2i(x,y-sy);
	glVertex2i(x+i,y-sy);
	glVertex2i(x+i,y);
	glEnd();

	glColor4ubv(color_back1);
	glBegin(GL_QUADS);
	glVertex2i(x+i,y);
	glVertex2i(x+i,y-sy);
	glVertex2i(x+sx,y-sy);
	glVertex2i(x+sx,y);
	glEnd();

	glLineWidth(2);
	glBegin(GL_LINE_STRIP);
	glVertex2i(x,y-1);
	glVertex2i(x,y-sy);
	glVertex2i(x+sx-1,y-sy);
	glVertex2i(x+sx-1,y-1);
	glVertex2i(x,y-1);
	glEnd();
	glLineWidth(1);

	glColor4ubv(color_text);
	y=s_screensizey-y+(sy-fontsize)/2;
	g_flyrender->draw_text(x+32,y,text,fontsize);

	static char str[256];
	sprintf(str,"%03i %%",progress);
	g_flyrender->draw_text(s_screensizex-4*fontsize,y,str,fontsize);

	end_draw2d();
	SwapBuffers(m_hdc);
}