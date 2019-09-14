#define GLEXT_PROTOTYPE
#include "../Fly3D.h"
#include "cpuid.h"

FLY_ENGINE_API flyEngine *g_flyengine=0;
FLY_ENGINE_API int g_processor_features=0;

FLY_ENGINE_API void fly_init_engine(HWND hWnd,HINSTANCE hInst,int appid)
{
	fly_free_engine();
	fly_set_engine(new flyEngine,hWnd,hInst,appid);
}

FLY_ENGINE_API void fly_set_engine(flyEngine *eng,HWND hWnd,HINSTANCE hInst,int appid)
{
	flyString str;

	g_flyengine=eng;
	g_flyengine->appid=appid;
	g_flyengine->hwnd=hWnd;
	g_flyengine->hinst=hInst;

	g_flyengine->console_printf("Fly3D ver %s",FLY_VERSION_STR);
	g_flyengine->console_printf("Engine Initialized");

	if (g_flyengine->load_ini()==0)
		g_flyengine->console_printf("Error loading Fly3D.ini!");		

	_p_info	pinfo;
	g_processor_features=_cpuid(&pinfo);
	if (g_processor_features)
	{
		g_flyengine->console_printf("Processor Features");
		g_flyengine->console_printf(" vendor: %s",pinfo.v_name);
		g_flyengine->console_printf(" cpu: %s (family %i model %i)",pinfo.model_name,pinfo.family,pinfo.model);

		str=" supports: ";
		if (g_processor_features&_CPU_FEATURE_MMX)
			str+="MMX ";
		if (g_processor_features&_CPU_FEATURE_SSE)
			str+="SSE ";	
		if (g_processor_features&_CPU_FEATURE_SSE2 )
			str+="SSE2 ";
		if (g_processor_features&_CPU_FEATURE_3DNOW )
			str+="3DNOW ";
		g_flyengine->console_printf(str);
	}
	if (g_flyrender)
	{
		InitGLextensions();
		g_flyengine->console_printf("Render Initialized");
		g_flyengine->console_printf(" vendor: %s",(char *)glGetString(GL_VENDOR));
		g_flyengine->console_printf(" render: %s",(char *)glGetString(GL_RENDERER));
		g_flyengine->console_printf(" version: %s",(char *)glGetString(GL_VERSION));
		g_flyengine->console_printf(" extensions: ");
		if (ARB_multitexture)
			g_flyengine->console_printf("    ARB_multitexture");
		if (ARB_multisample)
			g_flyengine->console_printf("    ARB_multisample");
		if (ARB_vertex_blend)
			g_flyengine->console_printf("    ARB_vertex_blend");
		if (ARB_texture_compression)
			g_flyengine->console_printf("    ARB_texture_compression");
		if (ARB_texture_cube_map)
			g_flyengine->console_printf("    ARB_texture_cube_map");
		if (ARB_texture_border_clamp)
			g_flyengine->console_printf("    ARB_texture_border_clamp");
		if (ARB_texture_env_add)
			g_flyengine->console_printf("    ARB_texture_env_add");
		if (ARB_texture_env_combine)
			g_flyengine->console_printf("    ARB_texture_env_combine");
		if (ARB_texture_env_crossbar)
			g_flyengine->console_printf("    ARB_texture_env_crossbar");
		if (ARB_texture_env_dot3)
			g_flyengine->console_printf("    ARB_texture_env_dot3");
		if (ARB_transpose_matrix)
			g_flyengine->console_printf("    ARB_transpose_matrix");
		if (EXT_compiled_vertex_array)
			g_flyengine->console_printf("    EXT_compiled_vertex_array");
		if (EXT_fog_coord)
			g_flyengine->console_printf("    EXT_fog_coord");
		if (EXT_vertex_shader)
			g_flyengine->console_printf("    EXT_vertex_shader");

		if (ATI_pn_triangles)
			g_flyengine->console_printf("    ATI_pn_triangles");
		if (ATI_envmap_bumpmap)
			g_flyengine->console_printf("    ATI_envmap_bumpmap");
		if (ATI_fragment_shader)
			g_flyengine->console_printf("    ATI_fragment_shader");
		if (ATI_vertex_array_object)
			g_flyengine->console_printf("    ATI_vertex_array_object");
		if (ATI_element_array)
			g_flyengine->console_printf("    ATI_element_array");
		
		if (NV_fence)
			g_flyengine->console_printf("    NV_fence");
		if (NV_evaluators)
			g_flyengine->console_printf("    NV_evaluators");
		if (NV_vertex_program)
			g_flyengine->console_printf("    NV_vertex_program");
		if (NV_texture_rectangle)
			g_flyengine->console_printf("    NV_texture_rectangle");
		if (NV_texture_shader)
			g_flyengine->console_printf("    NV_texture_shader");
		if (NV_texture_shader2)
			g_flyengine->console_printf("    NV_texture_shader2");
		if (NV_register_combiners)
			g_flyengine->console_printf("    NV_register_combiners");
		if (NV_register_combiners2)
			g_flyengine->console_printf("    NV_register_combiners2");
		if (NV_vertex_array_range)
			g_flyengine->console_printf("    NV_vertex_array_range");
		if (NV_vertex_array_range2)
			g_flyengine->console_printf("    NV_vertex_array_range2");

		if (SGIS_generate_mipmap)
			g_flyengine->console_printf("    SGIS_generate_mipmap");

		g_flytexcache->reset();
		g_flyengine->load_default_tex(g_flyengine->flysdkdatapath);
	}

	if (g_flydirectx)
	{
		g_flyengine->console_printf("DirectX Initialized");
		if (g_flydirectx->lpDI)
		{
			str="DirectInput:";
			if (g_flydirectx->lpKeyboard)
				str+=" keyboard";
			if (g_flydirectx->lpMouse)
				str+=" mouse";
#ifndef FLY_WIN_NT4_SUPORT
			if (g_flydirectx->lpJoystick)
				str+=" joystick";
#endif
			g_flyengine->console_printf(str);
		}

		if (g_flydirectx->lpDSound && g_flydirectx->lpDS3dPrimBuf)
		{
			WAVEFORMATEX format;
			g_flydirectx->lpDS3dPrimBuf->GetFormat(&format,sizeof(WAVEFORMATEX),0);
			g_flyengine->console_printf("DirectSound: %i x %iHz x %ibits",
				format.nChannels,format.nSamplesPerSec,format.wBitsPerSample);
		}

	}
}

FLY_ENGINE_API void fly_free_engine()
{
	delete g_flyengine;
	g_flyengine=0;
}

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

void fly_write_profile(LPTSTR section, LPCTSTR key, DWORD dwType, CONST BYTE *lpData, DWORD cbData)
{
	HKEY key1,key2,key3,key4;
	DWORD d;

	if (ERROR_SUCCESS==RegOpenKeyEx(HKEY_CURRENT_USER,"Software",0,KEY_READ,&key1))
	{
		if (ERROR_SUCCESS==RegCreateKeyEx(key1,FLY_REGISTRYKEY_PRIMARY,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&key2,&d))
		{
			if (ERROR_SUCCESS==RegCreateKeyEx(key2,FLY_REGISTRYKEY_SECONDARY,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&key3,&d))
			{
				if (ERROR_SUCCESS==RegCreateKeyEx(key3, section,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&key4,&d))
				{
					RegSetValueEx(key4, key, 0, dwType, lpData, cbData);
					RegCloseKey(key4);
				}
				RegCloseKey(key3);
			}
			RegCloseKey(key2);
		}
		RegCloseKey(key1);
	}
}

void flyEngine::load_default_tex(const char *path)
{
	int i;
	char str[256];
	flyPicture p;
	flyFile fp;

	glColor4ub(255,255,255,255);

	strcpy(str,path);
	strcat(str,"maps\\console.tga");
	if (fp.open(str))
	{
		if (p.LoadTGA(fp.buf,fp.len,1))
			flyRender::s_consolepic=g_flytexcache->add_tex("console.tga",p.sx,p.sy,p.bytespixel,p.buf,1+(flyTexCache::s_textruecolor?4:0));
		fp.close();
	}

	strcpy(str,path);
	strcat(str,"maps\\fonts.tga");
	if (fp.open(str))
	{
		if (p.LoadTGA(fp.buf,fp.len))
		{
			flyRender::s_fontspic=g_flytexcache->add_tex("fonts.tga",p.sx,p.sy,p.bytespixel,p.buf,3+(flyTexCache::s_textruecolor?4:0));
			flyRender::s_fontspicsize=p.sx;
			flyRender::s_fontspiccharsize=flyRender::s_fontspicsize/FLY_FONTS_NUM;
		}
		fp.close();
	}

	strcpy(str,path);
	strcat(str,"maps\\fonts.txt");
	if (fp.open(str))
		{
		for( i=0;i<256;i++ )
			flyRender::s_fontswidth[i]=fp.get_int();
		fp.close();
		}
	else 
		for( i=0;i<256;i++ )
			flyRender::s_fontswidth[i]=16;

}
