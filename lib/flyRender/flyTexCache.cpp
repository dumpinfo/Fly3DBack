#include "../flyRender.h"

FLY_RENDER_API flyTexCache *g_flytexcache=0;

int flyTexCache::s_texfilter=1;
int flyTexCache::s_texmipmap=1;
int flyTexCache::s_textruecolor=0;
int flyTexCache::s_texcompress=0;

flyTexCache::flyTexCache()
{
	ntex=0; 
	tex=0;
	vid=0;
	texinfo=0;
	memset(curtex,-1,sizeof(int)*FLY_MAXTEXUNITS);
	curtexunit=0;
}

flyTexCache::~flyTexCache()
{
	reset();
}

void flyTexCache::reset()
{
	if (tex)
		{
		glDeleteTextures(ntex,tex);
		delete[] tex;
		}
	for( int i=0;i<ntex;i++ )
		if (vid[i])
			delete vid[i];
	delete[] vid;
	delete[] texinfo;
	ntex=0;
	vid=0;
	tex=0;
	texinfo=0;
	memset(curtex,-1,sizeof(int)*FLY_MAXTEXUNITS);
}

int flyTexCache::alloc_tex(int num)
{
	unsigned *tmp1=new unsigned[ntex+num];
	if (tex)
		{
		memcpy(tmp1,tex,sizeof(unsigned)*ntex);
		delete[] tex;
		}
	tex=tmp1;

	flyVideo **tmp2=new flyVideo *[ntex+num];
	if (vid)
		{
		memcpy(tmp2,vid,sizeof(flyVideo *)*ntex);
		delete[] vid;
		}
	vid=tmp2;

	struct flyTexInfo *tmp3=new struct flyTexInfo[ntex+num];
	if (texinfo)
		{
		memcpy(tmp3,texinfo,sizeof(struct flyTexInfo)*ntex);
		delete[] texinfo;
		}
	texinfo=tmp3;

	glGenTextures(num,&tex[ntex]);

	ntex+=num;

	return ntex-num;
}

int flyTexCache::add_tex(int np,flyPicture **pic,int flags)
{
	int i,j;
	
	i=alloc_tex(np);

	for( j=0;j<np;j++)
		{
			vid[i+j]=0;
			texinfo[i+j].sx=pic[j]->sx;
			texinfo[i+j].sy=pic[j]->sy;
			texinfo[i+j].bytespixel=pic[j]->bytespixel;
			texinfo[i+j].flags=flags;
			strcpy(texinfo[i+j].name,pic[j]->name);
			load_tex(i+j,pic[j]->sx,pic[j]->sy,pic[j]->bytespixel,pic[j]->buf,flags);
		}
	
	return i;
}

int flyTexCache::add_tex(const char *name,int sx,int sy,int bytespixel,const unsigned char *buf,int flags)
{
	int i=alloc_tex(1);
	
	vid[i]=0;
	texinfo[i].sx=sx;
	texinfo[i].sy=sy;
	texinfo[i].bytespixel=bytespixel;
	texinfo[i].flags=flags;
	strcpy(texinfo[i].name,name);
	
	load_tex(i,sx,sy,bytespixel,buf,flags);

	return i;
}

int flyTexCache::add_tex(const char *name,flyVideo *video,int flags)
{
	int i=alloc_tex(1);
	
	vid[i]=video;
	texinfo[i].sx=video->resolution;
	texinfo[i].sy=video->resolution;
	texinfo[i].bytespixel=3;
	texinfo[i].flags=flags;
	strcpy(texinfo[i].name,name);
	
	load_tex(i,video->resolution,video->resolution,3,video->data,flags);

	return i;
}

void flyTexCache::build_mipmaps(int pic,int sx,int sy,int bytespixel,const unsigned char *buf)
{
	glBindTexture(GL_TEXTURE_2D,tex[pic]);

	unsigned char *data=new unsigned char[sx*sy*bytespixel],*d;
	memcpy(data,buf,sx*sy*bytespixel);
	
	int x,y,c,p,n,l=0,s;
	
	while(sx>1 || sy>1)
	{
		d=data;
		
		n=0;

		if (sx==1 || sy==1)
		{
			s=(sx==1?sy>>1:sx>>1);
			for( x=0;x<s;x++ )
			{
				for( c=0;c<bytespixel;c++ )
				{
					p=(unsigned)data[n]+(unsigned)data[n+bytespixel];
					*(d++)=(unsigned char)(p>>1);
					n++;
				}
				n+=bytespixel;
			}
		}
		else
		{
			s=sx*bytespixel;

			for( y=0;y<sy;y+=2 )
			{
				for( x=0;x<sx;x+=2 )
				{
					for( c=0;c<bytespixel;c++ )
					{
						p=(unsigned)data[n]+(unsigned)data[n+bytespixel]+
							(unsigned)data[n+s]+(unsigned)data[n+s+bytespixel];
						*(d++)=(unsigned char)(p>>2);
						n++;
					}

					n+=bytespixel;
				}
				n+=sx*bytespixel;
			}
		}
		
		sx>>=1;
		if (sx<1) sx=1;
		sy>>=1;
		if (sy<1) sy=1;
		l++;

		if ((texinfo[pic].flags&FLY_TEXFLAG_COMPRESSION) && ARB_texture_compression)
		{
			if (bytespixel==4)
				glTexImage2D(GL_TEXTURE_2D,l,GL_COMPRESSED_RGBA_ARB,sx,sy,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
			else 
				glTexImage2D(GL_TEXTURE_2D,l,GL_COMPRESSED_RGB_ARB,sx,sy,0,GL_RGB,GL_UNSIGNED_BYTE,data);
						
			int IsCompressed=0;
			glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_COMPRESSED_ARB,&IsCompressed);
			if (IsCompressed)
			{
				int CompressedInternalFormat=0;
				int CompressedImageSize=0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D,l,GL_TEXTURE_INTERNAL_FORMAT,&CompressedInternalFormat );
				glGetTexLevelParameteriv(GL_TEXTURE_2D,l,GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB,&CompressedImageSize );
				unsigned char *img = new unsigned char[CompressedImageSize];
				if( NULL != img )
				{
					glGetCompressedTexImageARB(GL_TEXTURE_2D,l,img);
					glCompressedTexImage2DARB(GL_TEXTURE_2D,l,CompressedInternalFormat,sx,sy,0,CompressedImageSize,img);
					delete[] img;
				}
			}

			continue;
		}

		if (bytespixel==4)
			glTexImage2D(GL_TEXTURE_2D,l,(texinfo[pic].flags&FLY_TEXFLAG_TRUECOLOR)?GL_RGBA:GL_RGBA4,sx,sy,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
		else 
			glTexImage2D(GL_TEXTURE_2D,l,(texinfo[pic].flags&FLY_TEXFLAG_TRUECOLOR)?GL_RGB:GL_RGB5,sx,sy,0,GL_RGB,GL_UNSIGNED_BYTE,data);
	}

	delete[] data;
}

void flyTexCache::load_tex(int pic,int sx,int sy,int bytespixel,const unsigned char *buf,int flags)
{
	update_tex(pic,sx,sy,bytespixel,buf);

	if (flyTexCache::s_texmipmap && (flags&FLY_TEXFLAG_MIPMAP))
	{
		build_mipmaps(pic,sx,sy,bytespixel,buf);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,flyTexCache::s_texfilter&&(flags&FLY_TEXFLAG_FILTER)?GL_LINEAR_MIPMAP_LINEAR:GL_NEAREST_MIPMAP_NEAREST);
	}
	else
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,flyTexCache::s_texfilter&&(flags&FLY_TEXFLAG_FILTER)?GL_LINEAR:GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,flyTexCache::s_texfilter&&(flags&FLY_TEXFLAG_FILTER)?GL_LINEAR:GL_NEAREST);
}

void flyTexCache::update_tex(int pic,int sx,int sy,int bytespixel,const unsigned char *buf)
{
	glBindTexture(GL_TEXTURE_2D,tex[pic]);
	
	if ((texinfo[pic].flags&FLY_TEXFLAG_COMPRESSION) && ARB_texture_compression)
	{
		if (bytespixel==4)
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGBA_ARB,sx,sy,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
		else 
			glTexImage2D(GL_TEXTURE_2D,0,GL_COMPRESSED_RGB_ARB,sx,sy,0,GL_RGB,GL_UNSIGNED_BYTE,buf);
	
		int IsCompressed=0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_COMPRESSED_ARB,&IsCompressed);
		if (IsCompressed)
		{
				int CompressedInternalFormat=0;
				int CompressedImageSize=0;
				glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_INTERNAL_FORMAT,&CompressedInternalFormat );
				glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB,&CompressedImageSize );
				unsigned char *img = new unsigned char[CompressedImageSize];
				if( NULL != img )
				{
					glGetCompressedTexImageARB(GL_TEXTURE_2D,0,img);
					glCompressedTexImage2DARB(GL_TEXTURE_2D,0,CompressedInternalFormat,sx,sy,0,CompressedImageSize,img);
					delete[] img;
					return;
				}
		}
		
		texinfo[pic].flags&=~FLY_TEXFLAG_COMPRESSION;
	}

	if (texinfo[pic].flags&FLY_TEXFLAG_BGR)
		if (bytespixel==4)
			glTexImage2D(GL_TEXTURE_2D,0,(texinfo[pic].flags&FLY_TEXFLAG_TRUECOLOR)?GL_RGBA:GL_RGBA4,sx,sy,0,GL_ABGR_EXT,GL_UNSIGNED_BYTE,buf);
		else 
			glTexImage2D(GL_TEXTURE_2D,0,(texinfo[pic].flags&FLY_TEXFLAG_TRUECOLOR)?GL_RGB:GL_RGB5,sx,sy,0,GL_BGR_EXT,GL_UNSIGNED_BYTE,buf);
	else
		if (bytespixel==4)
			glTexImage2D(GL_TEXTURE_2D,0,(texinfo[pic].flags&FLY_TEXFLAG_TRUECOLOR)?GL_RGBA:GL_RGBA4,sx,sy,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
		else 
			glTexImage2D(GL_TEXTURE_2D,0,(texinfo[pic].flags&FLY_TEXFLAG_TRUECOLOR)?GL_RGB:GL_RGB5,sx,sy,0,GL_RGB,GL_UNSIGNED_BYTE,buf);
}

void flyTexCache::update_subtex(int pic,int x,int y,int sx,int sy,int bytespixel,const unsigned char *buf)
{
	glBindTexture(GL_TEXTURE_2D,tex[pic]);
	if (bytespixel==4)
		glTexSubImage2D(GL_TEXTURE_2D,0,x,y,sx,sy,GL_RGBA,GL_UNSIGNED_BYTE,buf);
	else 
		glTexSubImage2D(GL_TEXTURE_2D,0,x,y,sx,sy,GL_RGB,GL_UNSIGNED_BYTE,buf);
}

int flyTexCache::find_tex(const char *file)
{
	int i;
	for( i=0;i<ntex;i++ )
		if (!stricmp(texinfo[i].name,file))
			break;

	if (i!=ntex)
		return i;
	else return -1;
}

int flyTexCache::get_tex(int pic,unsigned char *buf)
{
	int len=texinfo[pic].bytespixel*texinfo[pic].sx*texinfo[pic].sy;
	if (buf)
	{
	glBindTexture(GL_TEXTURE_2D,tex[pic]);
	if (texinfo[pic].bytespixel==3)
		glGetTexImage(GL_TEXTURE_2D,0,GL_RGB,GL_UNSIGNED_BYTE,buf);
	else
		glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,buf);
	}
	return len;
}

void flyTexCache::set_tex_flags()
{
	int i;
	for( i=0;i<ntex;i++ )
	{
		glBindTexture(GL_TEXTURE_2D,tex[i]);
		if (flyTexCache::s_texmipmap && (texinfo[i].flags&FLY_TEXFLAG_MIPMAP))
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
				flyTexCache::s_texfilter&&(texinfo[i].flags&FLY_TEXFLAG_FILTER)?GL_LINEAR_MIPMAP_LINEAR:GL_NEAREST_MIPMAP_NEAREST);
		else
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
				flyTexCache::s_texfilter&&(texinfo[i].flags&FLY_TEXFLAG_FILTER)?GL_LINEAR:GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
			flyTexCache::s_texfilter&&(texinfo[i].flags&FLY_TEXFLAG_FILTER)?GL_LINEAR:GL_NEAREST);
	}
}

void flyTexCache::clear_tex_state()
{
	int i=g_flyrender->m_numtextureunits-1;
	for( ;i>=0;i-- )
	{
		if (ARB_multitexture)
			glActiveTextureARB(GL_TEXTURE0_ARB+i);
		glDisable(GL_TEXTURE_2D);
	}
	memset(curtex,-1,sizeof(int)*FLY_MAXTEXUNITS);
	curtexunit=0;
}
