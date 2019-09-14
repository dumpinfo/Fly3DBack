/*! \file */

//! Dynamic hardware lights class
/*!
	This class holds information on dynamic hardware lights. 
	When a dynamic object receives a FLY_OBJMESSAGE_ILLUM message 
	it adds the light parameters to this class with the 'add_light' method. 
	When rendering, it calls the 'init_draw' method before drawing the mesh 
	to get the hardware lights in place. After finishing the render it calls 
	'end_draw' to reset the hardware lights.
*/
class FLY_ENGINE_API flyLightVertex
{
	public:
		int nlights;						//!< number of lights
		int frame;							//!< number of the frame the lights wore added
		flyVector pos[FLY_MAX_HWLIGHTS];	//!< position of each light
		flyVector color[FLY_MAX_HWLIGHTS];	//!< color of each light
		float radius[FLY_MAX_HWLIGHTS];		//!< radius of each light

	//! Default constructor
	flyLightVertex();
	
	//! Copy-constructor
	flyLightVertex(const flyLightVertex& in);

	//! Destructor
	~flyLightVertex();

	//! Atribuition operator
	void operator=(const flyLightVertex& in);

	//! Add a new dynamic light
	void add_light(const flyVector& p,const flyVector& c,float r);
	//! Add a new dynamic light sorted by distance to objpos
	void flyLightVertex::add_light(const flyVector& p,const flyVector& c,float r,const flyVector& objpos);
	//! Initialise drawing
	void init_draw(const flyBspObject *obj) const;
	//! Get the closest light to the given point
	int get_closest(const flyVector& p) const;
};

//! Light map picture class
/*!
	This class implements the light map texture used when drawing. 
	This texture can have several light map pictures in it as many 
	instances of the flyLightMap class will use a single flyLightMapPic.
*/
class FLY_ENGINE_API flyLightMapPic
{
	public:
		unsigned char *bmp;	//!< the light maps bitmap
		int sizex,			//!< x size of the picture (power of 2)
			sizey;			//!< y size of the picture (power of 2)
		int bytesx,			//!< number of bytes per line (sizex*bytespixel)
			bytesxy;		//!< total bytes of the bitmap (bytesx*sizey)
		int bytespixel;		//!< the light map bytes per pixel (3: RGB, 4: RGBA)

	//! Default Counstructor, passing the size in x and y, and the number of bytes per pixel
	flyLightMapPic(int sx,int sy,int bp=3);

	//! Default destructor
	virtual ~flyLightMapPic();
};

//! Light map class
/*!
	This class implements a light map. Several faces can share 
	the same light map and many light maps can share the same light map picture.
*/
class FLY_ENGINE_API flyLightMap
{
	protected:
		flyVector	v0,
					v1,
					v2,
					normal;
		float det;		
		float uv[3][2];	
	public:
		int lastupdate;		//!< last frame in which this light map was changed
		int pic;			//!< flyLightMapPic index
		int offsetx,		//!< x offset into the light map picture
			offsety;		//!< y offset into the light map picture
		int sizex,			//!< x size of the light map (no need for being power of 2)
			sizey;			//!< y size of the light map (no need for being power of 2)
		int facenum;		//!< face index for one face using this light map
		unsigned char *bmp;	//!< light map bitmap
		int bytespixel,		//!< number of bytes per pixel (3: RGB, 4: RGBA)
			bytesx,			//!< number of bytes per line (sizex*bytespixel)
			bytesxy;		//!< total bytes of the bitmap (bytesx*sizey)
		flyVector	d0,		//!< base value used for illumination calculations
					d1,		//!< base value used for illumination calculations
					d2;		//!< base value used for illumination calculations

	//! Precompute internal data required when mapping points from the light map space into world space and vice-versa
	void set_base(const flyFace *f,const flyLightMapPic *lmp, const flyVector& pos=flyVector(0,0,0));
	//! Map a point from the light_map (u,v) into world coordinates
	void map_point(float u, float v, flyVector &point) const;
	//! Map a point from the local light_map (u,v) into world coordinates
	void map_point_local(float u, float v, flyVector &p,flyVector &n) const;
	//! Dynamicaly illuminate the light map pixels based on the given parameters
	virtual void illum(const flyVector& pos,const flyVector& color,float radius,int shadows);
	//! Restore the light map pixels from flyLightMapPic
	void load(const flyLightMapPic *lmp);
	//! Save the light map to flyLightMapPic
	void save(flyLightMapPic *lmp) const;

	//! Default constructor
	flyLightMap(int f,int p,int x,int y,int sx,int sy,int bp=3);

	//! Default destructor
	virtual ~flyLightMap();
};
