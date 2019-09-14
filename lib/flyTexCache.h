/*! \file */

//! Maximum texture units
#define FLY_MAXTEXUNITS		4

//! Texture flags: texture filtering enabled
#define FLY_TEXFLAG_FILTER		1
//! Texture flags: mip-mapping enabled
#define FLY_TEXFLAG_MIPMAP		2
//! Texture flags: true color texture (24/32 bits/pixel)
#define FLY_TEXFLAG_TRUECOLOR	4
//! Texture flags: BGR texture format
#define FLY_TEXFLAG_BGR			8
//! Texture flags: ARB texture compression
#define FLY_TEXFLAG_COMPRESSION	16

//! texture info structure
struct flyTexInfo
{
	char name[240];	//!< texture filename
	int sx,			//!< x size in pixels
		sy,			//!< y size in pixels
		bytespixel,	//!< number of bytes per pixel (24 or 32)
		flags;		//!< texture flags
};

//!	Texture manager class
/*!
	This class is a texture cache (texture manager). The supplied 
	initialisation and release methods should be used to initialise and free 
	the global variable g_flytexcache that is the interface to this class. 
*/
class FLY_RENDER_API flyTexCache
{
	int alloc_tex(int num);
	void load_tex(int pic,int sx,int sy,int bytespixel,const unsigned char *buf,int flags);

public:
	//! Default constructor
	flyTexCache();
	//! Default destructor
	~flyTexCache();

	static int s_texfilter;			//!< texture filtering flag
	static int s_texmipmap;			//!< texture mip-mapping flag
	static int s_textruecolor;		//!< texture color depth flag: 0:high color (16 bits/pixel), 1:true color (24 bits/pixel)
	static int s_texcompress;		//!< texture compression flag

	int curtex[FLY_MAXTEXUNITS];	//!< current texture
	int curtexunit;					//!< current texture unit

	int ntex;			//!< number of texture
	unsigned *tex;		//!< OpenGL texture handles
	flyVideo **vid;		//!< video files
	flyTexInfo *texinfo; //!< texture info array

	//! Reset the manager
	void reset();
	//! Dinamically add a new texture to the cache, passing all info
	int add_tex(const char *name,int sx,int sy,int bytespixel,const unsigned char *buf,int flags);
	//! Dinamically add a new texture to the cache, passing a picture pointer list 
	int add_tex(int np,flyPicture **pic,int flags);
	//! Dinamically add a new video to the cache (video will belong to cache... do not delete it aftre this call)
	int add_tex(const char *name,flyVideo *video,int flags);
	//! Update the texture in the texture manager with a new pixel array
	void update_tex(int pic,int sx,int sy,int bytespixel,const unsigned char *buf);
	//! Update a part of a texture in the texture manager. Only the sub-texture pixels are passed. If 'x' and 'y' are 0 and 'sx' and 'sy' the size of the image, it will work just like the update_picture function.
	void update_subtex(int pic,int x,int y,int sx,int sy,int bytespixel,const unsigned char *buf);
	//! Returns the index of the desired texture
	int get_tex(int pic,unsigned char *buf);
	//! Find a texture given its filename
	int find_tex(const char *file);
	//! Sets the texture flags
	void set_tex_flags();
	//! Clear the texture state
	void clear_tex_state();
	//! Builds all mipmap levels for the given picture
	void build_mipmaps(int pic,int sx,int sy,int bytespixel,const unsigned char *buf);

	//! Select a texture unit
	inline void sel_unit(int u)
	{
		if (curtexunit!=u)
			{
			if (ARB_multitexture)
				{
				glActiveTextureARB(GL_TEXTURE0_ARB+u);
				glClientActiveTextureARB(GL_TEXTURE0_ARB+u);
				}
			curtexunit=u;
			}
	}
	//! Select a texture
	inline void sel_tex(int t)
	{
	if (curtex[curtexunit]!=t)
		{
		if (t==-1)
			glDisable(GL_TEXTURE_2D);
		else 
			{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,tex[t]);
			if (vid[t])
				vid[t]->update(g_flyrender->m_curtime);
			}
		curtex[curtexunit]=t;
		}
	}
	//! Select a texture and unit
	inline void sel_tex(int t,int u)
	{
	if (curtex[u]!=t)
		{
		sel_unit(u);
		if (t==-1)
			glDisable(GL_TEXTURE_2D);
		else 
			{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,tex[t]);
			if (vid[t])
				vid[t]->update(g_flyrender->m_curtime);
			}
		curtex[curtexunit]=t;
		}
	}
};

//! global flyTexCache instance
extern FLY_RENDER_API flyTexCache *g_flytexcache;
