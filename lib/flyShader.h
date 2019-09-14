/*! \file */

//! Shader function class
/*!
	This class implements a shader typical function, whose type can be:

	Sin
	Triangle
	Square
	Saw-tooth
	Inverse saw-tooth
*/
class FLY_ENGINE_API flyShaderFunc
{
	public:
		int type;		//!< function type (sin, triangle, square, saw-tooth or inverse saw-tooth)
		float args[4];	//!< function arguments (offset, amplitude, phase offset and rate)

	float eval() const;	//!< evaluate the function with the current arguments

	flyShaderFunc() :
		type(0)
	{
		args[0]=args[1]=args[2]=args[3]=0.0f;
	}
};

//! Shader pass class
/*!
	This class implements a shader pass, which can have a texture and various effeect applied to it. 
	A shader consists of various shader passes.
*/
class FLY_ENGINE_API flyShaderPass
{
	public:
		int flags;		//!< shader pass flags bitfield
		int tex;		//!< shader pass texture index
		int blendsrc;	//!< shader pass source blend factor (argument of blending function)
		int blenddst;	//!< shader pass destination blend factor (argument of blending function)

		int depthfunc;		//!< shader pass depth function
		int alphafunc;		//!< shader pass alpha function
		float alphafuncref;	//!< alpha function reference value

		int rgbgen;					//!< shader pass RGB generation mode
		flyShaderFunc rgbgen_func;	//!< RGB wave generation function

		int tcmod;							//!< shader pass texture coordinates modifier flag bitfield
		float tcmod_rotate;					//!< texture cordinates rotation factor
		float tcmod_scale[2];				//!< texture cordinates scaling factor in u and v
		float tcmod_scroll[2];				//!< texture cordinates scrolling factor in u and v
		flyShaderFunc tcmod_stretch_func;	//!< texture cordinates stretching function

		float anim_fps;								//!< shader pass animation frame rate
		int anim_numframes;							//!< number of frames in the animation
		int anim_frames[FLY_MAX_SHADERANIMFRAMES];	//!< each animation frame

		flyString vp;			//!< vertex program file
		flyShaderFunc vpfunc;	//!< vertex program function
		unsigned vpid;			//!< vertex program id

	//! Default constructor
	flyShaderPass();

	//! Default destructor
	virtual ~flyShaderPass();
	
	//! Set the pass vertex program
	void set_vp(const char *file);
	//! Load a shader pass from a shader file
	void load(flyFile *fp,const char *section,int i);
	//! Save a shader pass to a shader file
	void save(FILE *fp,int i) const;
	//! Set the shader pass states concerning depth, alpha and RGB generation functions
	void set_state_1(const flyVector& color);
	//! Set the shader pass states concerning animations, texture coordinates modifiers, texture clamping and environment mapping
	void set_state_2();
};

//! Shader class
/*!
	This class implements a multi-pass shader, that can be applied over any surface of the geometry.
*/
class FLY_ENGINE_API flyShader : public flyBaseObject
{
	int curpass;

	public:
		int flags;									//!< shader flags bitfield
		int npass;									//!< number of passes
		flyShaderPass pass[FLY_MAX_SHADERPASSES];	//!< shader passes array
		flyVector color;							//!< face color for radiosity
		float selfillum;							//!< face self illumination for radiosity

	//! Default constructor
	flyShader();

	//! Default destructor
	virtual ~flyShader();

	//! Set the shader state to pass 'cp'
	int set_state(int cp);
	//! Restore the shader original state
	int restore_state();
	//! Load the shader from a file
	void load(flyFile *fp,const char *section);
	//! Save the shader to a file
	void save(const char *filename,int i) const;
};
