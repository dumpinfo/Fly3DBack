/*! \file */

#define FLY_REGISTRYKEY_PRIMARY			"Paralelo"	//!< Fly3D main Registry folder
#define FLY_REGISTRYKEY_SECONDARY		"Fly3D"		//!< Fly3D Registry sub-folder
#define FLY_VERSION_STR					"2.0"		//!< Current version in string format
#define FLY_VERSION_NUM					230			//!< Current version number

#define FLY_APPID_NONE					0		//!< Application id: no application
#define FLY_APPID_FLYFRONTEND			1		//!< Application id: flyFrontend
#define FLY_APPID_FLYEDITOR				2		//!< Application id: flyEditor
#define FLY_APPID_FLYSERVER				4		//!< Application id: flyServer
#define FLY_APPID_FLYOCX				5		//!< Application id: flyActiveX
#define FLY_APPID_FLYSHADER				6		//!< Application id: flyShader
#define FLY_APPID_FLYBUILD				7		//!< Application id: flyBuild

#define FLY_FILEID_FMP					0xfaa	//!< File id: FMP file
#define FLY_FILEID_BSP					0xfab	//!< File id: BSP file
#define FLY_FILEID_PVS					0xfac	//!< File id: PVS file
#define FLY_FILEID_LMP					0xfad	//!< File id: lightmap file
#define FLY_FILEID_F3D					0xfae	//!< File id: F3D file
#define FLY_FILEID_K3D					0xfaf	//!< File id: K3D file

#define FLY_TYPE_NONE					0		//!< Base object type: Undefined
#define FLY_TYPE_STATICMESH				1		//!< Base object type: Static mesh
#define FLY_TYPE_MESH					2		//!< Base object type: Standard mesh
#define FLY_TYPE_VERTEXMESH				3		//!< Base object type: Vertex animated mesh
#define FLY_TYPE_SKELETONMESH			4		//!< Base object type: Skeleton animated mesh
#define FLY_TYPE_SOUND					5		//!< Base object type: Sound
#define FLY_TYPE_BEZIERCURVE			6		//!< Base object type: Bezier curve

#define FLY_NUMDEFAULTPARAM				12		//!< Number of the engine's default parameters

#define FLY_MAX_OBJDRAWLAYERS			8		//!< Maximum number of drawing layers
#define FLY_MAX_BSPSTACK				64		//!< Maximum number of entries in the BSP stack

#define FLY_MAX_MESHANIMS				256		//!< Maximum number of animations in a mesh
#define FLY_MAX_MESHBONES				256		//!< Maximum number of bones in a skeleton mesh
#define FLY_MAX_MESHSTRLEN				32		//!< Maximum length of strings in the mesh file format

#define FLY_MAX_SHADERPASSES			8		//!< Maximum number of passes in each shader
#define FLY_MAX_SHADERANIMFRAMES		16		//!< Maximum number of shader animation map frames

#define FLY_MAX_HWLIGHTS				4		//!< Maximum number of hardware lights

#define FLY_MESSAGE_INITSCENE			1000	//!< Default message: Initialise scene
#define FLY_MESSAGE_UPDATESCENE			1001	//!< Default message: Update scene
#define FLY_MESSAGE_DRAWSCENE			1002	//!< Default message: Draw scene (3D drawing)
#define FLY_MESSAGE_DRAWTEXT			1003	//!< Default message: 2D drawing
#define FLY_MESSAGE_CLOSESCENE			1004	//!< Default message: Close scene
#define FLY_MESSAGE_MPMESSAGE			1005	//!< Default message: Multiplayer general
#define FLY_MESSAGE_MPUPDATE			1006	//!< Default message: Multiplayer update
#define FLY_MESSAGE_DROPDATA			1007	//!< Default message: Data drop into engine
#define FLY_MESSAGE_FULLSCREEN			1008	//!< Default message: Inverts full screen state

#define FLY_OBJMESSAGE_CHANGEPARAM		2000	//!< Object message: Parameter change
#define FLY_OBJMESSAGE_ILLUM			2001	//!< Object message: Search for lights and self-illuminate
#define FLY_OBJMESSAGE_STATICILLUM		2002	//!< Object message: Search for static lights and self-illuminate
#define FLY_OBJMESSAGE_DYNILLUM			2003	//!< Object message: Search for dynamic lights and self-illuminate
#define FLY_OBJMESSAGE_DAMAGE			2004	//!< Object message: Splash damage
#define FLY_OBJMESSAGE_DAMAGECONTACT	2005	//!< Object message: Contact damage
#define FLY_OBJMESSAGE_LIGHT			2006	//!< Object message: Illuminate (generate a light source on current position
#define FLY_OBJMESSAGE_TRIGGER			2007	//!< Object message: Generic trigger

#define FLY_MESH_FACENORM				1		//!< Mesh bitfield: face normal bit
#define FLY_MESH_VERTNORM				2		//!< Mesh bitfield: vertex normal bit
#define FLY_MESH_BBOX					4		//!< Mesh bitfield: bounding box bit
#define FLY_MESH_OCTREE					8		//!< Mesh bitfield: octree bit
#define FLY_MESH_ALL					15		//!< All bitfields set to one

#define FLY_PATCHEVAL_POINT				1		//!< Patch evaluation bitfield: point bit
#define FLY_PATCHEVAL_TEXTCOORD			2		//!< Patch evaluation bitfield: texture coordinates bit
#define FLY_PATCHEVAL_NORMAL			4		//!< Patch evaluation bitfield: normal bit

#define FLY_FACETYPE_LARGE_POLYGON		0		//!< Face type: Face is large convex polygon made of any number of edges
#define FLY_FACETYPE_BEZIER_PATCH		1		//!< Face type: Face is a bi-quadratic Bezier patch
#define FLY_FACETYPE_TRIANGLE_MESH		2		//!< Face type: Face is a generic triangle mesh
#define FLY_FACETYPE_TRIANGLE_STRIP		3		//!< Face type: Face is a triangle strip
#define FLY_FACETYPE_TRIANGLE_FAN		4		//!< Face type: Face is a triangle fan

#define FLY_FACEFLAG_DETAIL				0		//!< Face flag: detail face
#define FLY_FACEFLAG_STRUCTURAL			1		//!< Face flag: srtructural face
#define FLY_FACEFLAG_LANDSCAPE			2		//!< Face flag: landscape face

#define FLY_CONSOLE_MAXLINES			512		//!< Maximum number of lines in console
#define FLY_CONSOLE_COL					40		//!< Console column width in screen pixels
#define FLY_CONSOLE_LINE				30		//!< Console line width in screen pixels

//! PVS testing
#define FLY_PVS_TEST(from,to) \
        (*(pvs + (from)*pvsrowsize + ((to)>>3)) & (1 << ((to) & 7)))

#define FLY_GLOBAL_PVS_TEST(from,to) \
        (*(g_flyengine->pvs + (from)*g_flyengine->pvsrowsize + ((to)>>3)) & (1 << ((to) & 7)))

// flyShader flags
#define FLY_SHADER_NOCULL				1		//!< Shader bitfield: No culling bit
#define FLY_SHADER_TRANSPARENT			2		//!< Shader bitfield: Transparency bit
#define FLY_SHADER_NODEPTHWRITE			4		//!< Shader bitfield: No depth buffer writing bit
#define FLY_SHADER_SKY					8		//!< Shader bitfield: Sky bit
#define FLY_SHADER_NOCOLLIDE			16		//!< Shader bitfield: No colliding bit
#define FLY_SHADER_NOSHADOWS			32		//!< Shader bitfield: No shadows bit

// flyShaderPass flags
#define FLY_SHADER_LIGHTMAP				1		//!< Shader pass bitfield: Lightmap bit
#define FLY_SHADER_BLEND				2		//!< Shader pass bitfield: Blending bit
#define FLY_SHADER_ALPHAFUNC			8		//!< Shader pass bitfield: Alpha function bit
#define FLY_SHADER_TCMOD				16		//!< Shader pass bitfield: Texture coordinates modifier bit
#define FLY_SHADER_ANIMMAP				32		//!< Shader pass bitfield: Animation map bit
#define FLY_SHADER_TCGEN_ENV			64		//!< Shader pass bitfield: Environment bit
#define FLY_SHADER_TEXCLAMP				128		//!< Shader pass bitfield: Texture clamping bit
#define FLY_SHADER_INVERTCULLING		256		//!< Shader pass bitfield: Invert backface culling bit

// transform functions
#define FLY_SHADERFUNC_SIN				1		//!< Shader function: Sin
#define FLY_SHADERFUNC_TRIANGLE			2		//!< Shader function: Triangle
#define FLY_SHADERFUNC_SQUARE			3		//!< Shader function: Square
#define FLY_SHADERFUNC_SAWTOOTH			4		//!< Shader function: Saw-tooth
#define FLY_SHADERFUNC_INVERSESAWTOOTH	5		//!< Shader function: Inverse saw-tooth

// gen functions
#define FLY_SHADERGEN_DEFAULT			-1		//!< Shader RGB generation: Default
#define FLY_SHADERGEN_IDENTITY			0		//!< Shader RGB generation: Identity
#define FLY_SHADERGEN_WAVE				1		//!< Shader RGB generation: Wave function
#define FLY_SHADERGEN_VERTEX			2		//!< Shader RGB generation: Vertex
#define FLY_SHADERGEN_REFLECTANCE		3		//!< Shader RGB generation: Reflectance
#define FLY_SHADERGEN_REFLECTANCEWAVE	4		//!< Shader RGB generation: Reflectance * Wave

// tcmod functions
#define FLY_SHADERTCMOD_SCALE			2		//!< Shader texture coordinates modifier bitfield: Scaling
#define FLY_SHADERTCMOD_ROTATE			4		//!< Shader texture coordinates modifier bitfield: Rotation
#define FLY_SHADERTCMOD_SCROLL			8		//!< Shader texture coordinates modifier bitfield: Scrolling
#define FLY_SHADERTCMOD_STRETCH			64		//!< Shader texture coordinates modifier bitfield: Stretching

#ifndef RGB_MAKE

/*
 * Format of RGBA colors is
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    alpha      |      red      |     green     |     blue      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define RGBA_GETALPHA(rgb)      ((rgb) >> 24)
#define RGBA_GETRED(rgb)        (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)      (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)       ((rgb) & 0xff)
#define RGBA_MAKE(r, g, b, a)   ((DWORD) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#define RGBA_SETALPHA(rgba, x) (((x) << 24) | ((rgba) & 0x00ffffff))
#define RGBA_TORGB(rgba)       ((DWORD) ((rgba) & 0xffffff))

#define D3DRGBA(r, g, b, a) \
    (   (((DWORD)((a) * 255)) << 24) | (((DWORD)((r) * 255)) << 16) \
    |   (((DWORD)((g) * 255)) << 8) | (DWORD)((b) * 255) \
    )

/*
 * Format of RGB colors is
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    ignored    |      red      |     green     |     blue      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define RGB_GETRED(rgb)         (((rgb) >> 16) & 0xff)
#define RGB_GETGREEN(rgb)       (((rgb) >> 8) & 0xff)
#define RGB_GETBLUE(rgb)        ((rgb) & 0xff)
#define RGB_MAKE(r, g, b)       ((DWORD) (((r) << 16) | ((g) << 8) | (b)))
#define RGB_TORGBA(rgb)        ((DWORD) ((rgb) | 0xff000000))

#define D3DRGB(r, g, b) \
    (0xff000000L | ( ((DWORD)((r) * 255)) << 16) | (((DWORD)((g) * 255)) << 8) | (DWORD)((b) * 255))

#endif
