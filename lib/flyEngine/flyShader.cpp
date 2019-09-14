#include "..\..\lib\Fly3D.h"

flyShader::flyShader() :
	flags(0),
	npass(0),
	color(1),
	selfillum(0)
{
}

flyShader::~flyShader()
{
}

flyShaderPass::flyShaderPass() : 
	flags(6),
	tex(-1),
	blendsrc(770),
	blenddst(770),
	depthfunc(515),
	alphafunc(0),
	alphafuncref(0.5f),
	rgbgen(2),
	tcmod(0),
	tcmod_rotate(0),
	anim_fps(10.0f),
	anim_numframes(0),
	vpid(0)
{
	tcmod_scale[0]=tcmod_scale[1]=1.0f;
	tcmod_scroll[0]=tcmod_scroll[1]=0.0f;
	for( int i=0;i<FLY_MAX_SHADERANIMFRAMES;i++ )
		anim_frames[i]=-1;
}

flyShaderPass::~flyShaderPass()
{
	if (vpid)
		glDeleteProgramsNV(1,&vpid);
}

float flyShaderFunc::eval() const 
{
    float x, y;

    /* Evaluate a number of time based periodic functions */
    /* y = args[0] + args[1] * func( (time + arg[2]) * arg[3] ) */
    
    x = (g_flyengine->cur_time_float + args[2]) * args[3];
    x -= (int)x;

    switch (type)
    {
	case FLY_SHADERFUNC_SIN:
	    y = (float)sin(x * 6.283185307179586476925286766559f);
	    break;
	    
	case FLY_SHADERFUNC_TRIANGLE:
	    if (x < 0.5f)
			y = 4.0f * x - 1.0f;
	    else
			y = -4.0f * x + 3.0f;
	    break;
	    
	case FLY_SHADERFUNC_SQUARE:
	    if (x < 0.5f)
			y = 1.0f;
	    else
			y = -1.0f;
	    break;
	    
	case FLY_SHADERFUNC_SAWTOOTH:
	    y = x;
	    break;
	    
	case FLY_SHADERFUNC_INVERSESAWTOOTH:
	    y = 1.0f - x;
	    break;
    }

    return y * args[1] + args[0];
}

int flyShader::set_state(int cp)
{
	if (g_flyengine->shadermask!=-1)
		while((g_flyengine->shadermask&(1<<cp))==0) 
		{
			cp++;
			if (cp>=npass)
				return -1;
		}

	curpass=cp;

	flyShaderPass *p=&pass[curpass];

	if (curpass==0)
	{
	if (flags&FLY_SHADER_NOCULL)
		glDisable(GL_CULL_FACE);
	else 
		glEnable(GL_CULL_FACE);

	if (flags&FLY_SHADER_NODEPTHWRITE)
		glDepthMask(GL_FALSE);
    else
		glDepthMask(GL_TRUE);
	}

	p->set_state_1(color);

	if (p->flags & FLY_SHADER_LIGHTMAP)
		{
		if (curpass==0 && npass>1 && 
			g_flyengine->multitexture && 
			g_flyrender->m_numtextureunits>1)
			{
			g_flytexcache->sel_tex(p[1].tex,1);
			g_flytexcache->sel_unit(0);
			return 2;
			}
		return 1;
		}

	p->set_state_2();

	return 0;
}

int flyShader::restore_state()
{
	flyShaderPass *p=&pass[curpass];

   	if (p->vpid)
		glDisable(GL_VERTEX_PROGRAM_NV);

	if (p->flags&FLY_SHADER_INVERTCULLING)
		glCullFace(GL_BACK);

	if (p->rgbgen == FLY_SHADERGEN_VERTEX)
		glDisableClientState(GL_COLOR_ARRAY);

	if (p->flags & FLY_SHADER_LIGHTMAP)
		{
		if (curpass==0 && npass>1 && 
			g_flyengine->multitexture && g_flyrender->m_numtextureunits>1)
			{
			g_flytexcache->sel_tex(-1,1);
			g_flytexcache->sel_unit(0);
			return 2;
			}
		return 1;
		}
    
	if (p->flags & FLY_SHADER_TCMOD)
		glLoadIdentity();

	if (p->flags & FLY_SHADER_TCGEN_ENV)
		{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		}
	return 1;
}

void flyShaderPass::set_state_1(const flyVector& color)
{
	glDepthFunc(depthfunc);

	if (flags&FLY_SHADER_INVERTCULLING)
		glCullFace(GL_FRONT);

	if (flags & FLY_SHADER_BLEND)
	{
		glEnable(GL_BLEND);
		glBlendFunc(blendsrc, blenddst);
	}
	else
		glDisable(GL_BLEND);

	if (flags & FLY_SHADER_ALPHAFUNC)
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(alphafunc, alphafuncref);
	}
	else
		glDisable(GL_ALPHA_TEST);

	if (rgbgen == FLY_SHADERGEN_IDENTITY)
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	else 
	if (rgbgen == FLY_SHADERGEN_REFLECTANCE)
		glColor4fv(&color.x);
	else
	if (rgbgen == FLY_SHADERGEN_REFLECTANCEWAVE)
	{
		float rgb = rgbgen_func.eval();
		glColor4f(rgb*color.x, rgb*color.y, rgb*color.z, color.w);
	}
	else
	if (rgbgen == FLY_SHADERGEN_WAVE)
	{
		float rgb = rgbgen_func.eval();
		glColor4f(rgb, rgb, rgb, 1.0f);
	}
	else 
	if (rgbgen == FLY_SHADERGEN_VERTEX)
		glEnableClientState(GL_COLOR_ARRAY);
	else
	if (rgbgen == FLY_SHADERGEN_DEFAULT)
		glColor4fv(&g_flyengine->shadercolor.x);

	if (vpid && g_flyengine->vertprog)
	{
		glBindProgramNV(GL_VERTEX_PROGRAM_NV, vpid);
		glEnable(GL_VERTEX_PROGRAM_NV);
		if (vpfunc.type)
		{
			float f=vpfunc.eval();
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 24,vpfunc.args[0],vpfunc.args[1],vpfunc.args[2],vpfunc.args[3]);
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 25,f,f,f,0);
		}
		else
		{
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 24,0,0,0,0);
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 25,0,0,0,0);
		}
		glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 26,g_flyengine->cur_time_float,g_flyengine->cur_time_float,g_flyengine->cur_time_float,0);
	}
}

void flyShaderPass::set_state_2()
{
	if ((flags & FLY_SHADER_ANIMMAP) && anim_numframes>0)
		g_flytexcache->sel_tex(anim_frames[(int)(anim_fps*g_flyengine->cur_time_float)%anim_numframes]);
	else g_flytexcache->sel_tex(tex);
		
	if (flags & FLY_SHADER_TEXCLAMP)
		{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}
	else
		{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}

	if (flags & FLY_SHADER_TCGEN_ENV)
		{
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		}

	if (flags & FLY_SHADER_TCMOD)
		{
		glLoadIdentity();

		glTranslatef(0.5f, 0.5f, 0.0f);
		
		if (tcmod & FLY_SHADERTCMOD_ROTATE)
			glRotatef(tcmod_rotate * g_flyengine->cur_time_float, 0.0f, 0.0f, 1.0f);

		if (tcmod & FLY_SHADERTCMOD_SCALE)
			glScalef(tcmod_scale[0], tcmod_scale[1], 1.0f);

		if (tcmod & FLY_SHADERTCMOD_STRETCH)
			{
			float y = tcmod_stretch_func.eval();
			glScalef(1.0f/y, 1.0f/y, 1.0f);
			}

		if (tcmod & FLY_SHADERTCMOD_SCROLL)
			if (tcmod & FLY_SHADERTCMOD_SCALE)
				glTranslatef(
					FLY_FPBITS(tcmod_scroll[0])?tcmod_scroll[0]*(float)fmod(g_flyengine->cur_time_float,1.0f/(tcmod_scroll[0]*tcmod_scale[0])):0.0f,
					FLY_FPBITS(tcmod_scroll[1])?tcmod_scroll[1]*(float)fmod(g_flyengine->cur_time_float,1.0f/(tcmod_scroll[1]*tcmod_scale[1])):0.0f,
					0.0f);
			else
				glTranslatef(
					FLY_FPBITS(tcmod_scroll[0])?tcmod_scroll[0]*(float)fmod(g_flyengine->cur_time_float,1.0f/tcmod_scroll[0]):0.0f,
					FLY_FPBITS(tcmod_scroll[1])?tcmod_scroll[1]*(float)fmod(g_flyengine->cur_time_float,1.0f/tcmod_scroll[1]):0.0f,
					0.0f);

		glTranslatef(-0.5f, -0.5f, 0.0f);
		}
}

void flyShader::load(flyFile *fp,const char *section)
{
	flyString str;
	flags=0;
	fp->get_profile_string(section,"flags",str);
	sscanf(str,"%i",&flags);
	color.vec(1);
	fp->get_profile_string(section,"color",str);
	sscanf(str,"%f %f %f %f",&color.x,&color.y,&color.z,&color.w);
	selfillum=0;
	fp->get_profile_string(section,"selfillum",str);
	sscanf(str,"%f",&selfillum);
	npass=0;
	fp->get_profile_string(section,"npass",str);
	sscanf(str,"%i",&npass);
	int i;
	for( i=0;i<npass;i++ )
		pass[i].load(fp,section,i);
}

void flyShader::save(const char *filename,int i) const 
{
	FILE *fp=fopen(filename,"r+t");
	if (fp)
	{
		fseek(fp,0,SEEK_END);

		fprintf(fp,"[shader%i]\n",i);
		fprintf(fp,"longname=%s\n",(const char *)long_name);
		fprintf(fp,"flags=%i\n",flags);
		if (FLY_FPBITS(color.x)!=FLY_FPONEBITS ||
			FLY_FPBITS(color.y)!=FLY_FPONEBITS ||
			FLY_FPBITS(color.z)!=FLY_FPONEBITS ||
			FLY_FPBITS(color.w)!=FLY_FPONEBITS)
			fprintf(fp,"color=%f %f %f %f\n",color.x,color.y,color.z,color.w);
		if (FLY_FPBITS(selfillum)!=0)
			fprintf(fp,"selfillum=%f\n",selfillum);
		fprintf(fp,"npass=%i\n",npass);
		
		for( int j=0;j<npass;j++ )
			pass[j].save(fp,j);

		fprintf(fp,"\n");
	}
	fclose(fp);
}

void flyShaderPass::load(flyFile *fp,const char *section,int i)
{
	flyString str;
	static char key[256];
	int j,k;
	
	sprintf(key,"pass%i.",i);
	j=strlen(key);

	flags=0;
	strcpy(&key[j],"flags");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i",&flags);

	strcpy(&key[j],"tex");
	fp->get_profile_string(section,key,str);
	tex=g_flyengine->get_picture(str,flyRender::s_texdroplevel);

	blendsrc=GL_ONE;
	blenddst=GL_ONE;
	strcpy(&key[j],"blend");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i %i",&blendsrc,&blenddst);

	depthfunc=GL_LEQUAL;
	strcpy(&key[j],"depthfunc");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i",&depthfunc);

	alphafunc=GL_GEQUAL;
	strcpy(&key[j],"alphafunc");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i",&alphafunc);

	alphafuncref=0.5f;
	strcpy(&key[j],"alphafuncref");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%f",&alphafuncref);

	rgbgen=0;
	strcpy(&key[j],"rgbgen");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i",&rgbgen);

	rgbgen_func.type=1;
	rgbgen_func.args[0]=rgbgen_func.args[2]=0;
	rgbgen_func.args[1]=rgbgen_func.args[3]=1;
	strcpy(&key[j],"rgbgen_func");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i %f %f %f %f",&rgbgen_func.type,
		&rgbgen_func.args[0],&rgbgen_func.args[1],
		&rgbgen_func.args[2],&rgbgen_func.args[3]);

	tcmod=0;
	strcpy(&key[j],"tcmod");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i",&tcmod);

	tcmod_scale[0]=tcmod_scale[1]=1.0f;
	strcpy(&key[j],"tcmod_scale");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%f %f",&tcmod_scale[0],&tcmod_scale[1]);

	tcmod_scroll[0]=tcmod_scroll[1]=0.0f;
	strcpy(&key[j],"tcmod_scroll");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%f %f",&tcmod_scroll[0],&tcmod_scroll[1]);

	tcmod_rotate=0.0f;
	strcpy(&key[j],"tcmod_rotate");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%f",&tcmod_rotate);

	tcmod_stretch_func.type=1;
	tcmod_stretch_func.args[0]=tcmod_stretch_func.args[2]=0;
	tcmod_stretch_func.args[1]=tcmod_stretch_func.args[3]=1;
	strcpy(&key[j],"tcmod_stretch_func");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i %f %f %f %f",&tcmod_stretch_func.type,
		&tcmod_stretch_func.args[0],&tcmod_stretch_func.args[1],
		&tcmod_stretch_func.args[2],&tcmod_stretch_func.args[3]);

	anim_numframes=0;
	strcpy(&key[j],"anim_numframes");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i",&anim_numframes);

	anim_fps=1.0f;
	strcpy(&key[j],"anim_fps");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%f",&anim_fps);

	for( k=0;k<anim_numframes;k++ )
	{
	sprintf(&key[j],"anim_frame%i",k);
	fp->get_profile_string(section,key,str);
	anim_frames[k]=g_flyengine->get_picture(str,flyRender::s_texdroplevel);
	}

	sprintf(&key[j],"vp");
	fp->get_profile_string(section,key,str);
	set_vp(str);

	strcpy(&key[j],"vpfunc");
	fp->get_profile_string(section,key,str);
	sscanf(str,"%i %f %f %f %f",&vpfunc.type,
		&vpfunc.args[0],&vpfunc.args[1],
		&vpfunc.args[2],&vpfunc.args[3]);
}

void flyShaderPass::save(FILE *fp,int i) const
{
	static char key[256];
	int j,k;
	
	sprintf(key,"pass%i.",i);
	j=strlen(key);

	strcpy(&key[j],"flags");
	fprintf(fp,"%s=%i\n",key,flags);

	strcpy(&key[j],"tex");
	if (tex==-1)
		fprintf(fp,"%s=\n",key);
	else 
		fprintf(fp,"%s=%s\n",key,g_flytexcache->texinfo[tex].name);

	if (flags&FLY_SHADER_BLEND)
	{
		strcpy(&key[j],"blend");
		fprintf(fp,"%s=%i %i\n",key,blendsrc,blenddst);
	}

	strcpy(&key[j],"depthfunc");
	fprintf(fp,"%s=%i\n",key,depthfunc);

	if (flags&FLY_SHADER_ALPHAFUNC)
	{
		strcpy(&key[j],"alphafunc");
		fprintf(fp,"%s=%i\n",key,alphafunc);

		strcpy(&key[j],"alphafuncref");
		fprintf(fp,"%s=%f\n",key,alphafuncref);
	}

	strcpy(&key[j],"rgbgen");
	fprintf(fp,"%s=%i\n",key,rgbgen);

	if (rgbgen==FLY_SHADERGEN_WAVE)
	{
		strcpy(&key[j],"rgbgen_func");
		fprintf(fp,"%s=%i %f %f %f %f\n",key,
			rgbgen_func.type,
			rgbgen_func.args[0],rgbgen_func.args[1],
			rgbgen_func.args[2],rgbgen_func.args[3]);
	}

	if (flags&FLY_SHADER_TCMOD)
	{
		strcpy(&key[j],"tcmod");
		fprintf(fp,"%s=%i\n",key,tcmod);
		
		if (tcmod&FLY_SHADERTCMOD_SCALE)
		{
			strcpy(&key[j],"tcmod_scale");
			fprintf(fp,"%s=%f %f\n",key,tcmod_scale[0],tcmod_scale[1]);
		}

		if (tcmod&FLY_SHADERTCMOD_SCROLL)
		{
			strcpy(&key[j],"tcmod_scroll");
			fprintf(fp,"%s=%f %f\n",key,tcmod_scroll[0],tcmod_scroll[1]);
		}

		if (tcmod&FLY_SHADERTCMOD_ROTATE)
		{
			strcpy(&key[j],"tcmod_rotate");
			fprintf(fp,"%s=%f\n",key,tcmod_rotate);
		}

		if (tcmod&FLY_SHADERTCMOD_STRETCH)
		{
			strcpy(&key[j],"tcmod_stretch_func");
			fprintf(fp,"%s=%i %f %f %f %f\n",key,
				tcmod_stretch_func.type,
				tcmod_stretch_func.args[0],tcmod_stretch_func.args[1],
				tcmod_stretch_func.args[2],tcmod_stretch_func.args[3]);
		}
	}

	if (flags&FLY_SHADER_ANIMMAP)
	{
		strcpy(&key[j],"anim_numframes");
		fprintf(fp,"%s=%i\n",key,anim_numframes);

		strcpy(&key[j],"anim_fps");
		fprintf(fp,"%s=%f\n",key,anim_fps);

		for( k=0;k<anim_numframes;k++ )
		{
			sprintf(&key[j],"anim_frame%i",k);
			if (anim_frames[k]==-1)
				fprintf(fp,"%s=\n",key);
			else 
				fprintf(fp,"%s=%s\n",key,g_flytexcache->texinfo[anim_frames[k]].name);
		}
	}

	if (vp[0]!=0)
	{
		strcpy(&key[j],"vp");
		fprintf(fp,"%s=%s\n",key,(const char *)vp);
		if (vpfunc.type)
		{
		strcpy(&key[j],"vpfunc");
		fprintf(fp,"%s=%i %f %f %f %f\n",key,
			vpfunc.type,
			vpfunc.args[0],vpfunc.args[1],
			vpfunc.args[2],vpfunc.args[3]);
		}
	}

}

void flyShaderPass::set_vp(const char *file)
{
	if (NV_vertex_program==0)
		return;

	if (vpid)
		glDeleteProgramsNV(1,&vpid);

	flyString str=g_flyengine->flyfilepath+"programs\\"+file;
	flyFile vpfile;
	if (file[0] && vpfile.open(str))
		{
		glGenProgramsNV(1,&vpid);
		glBindProgramNV(GL_VERTEX_PROGRAM_NV,vpid);
	    
		glLoadProgramNV(GL_VERTEX_PROGRAM_NV,vpid,vpfile.len,(unsigned char *)vpfile.buf);

		vpfile.close();
		vp=file;
		}
	else
		vp.set_char(0,0);
}
