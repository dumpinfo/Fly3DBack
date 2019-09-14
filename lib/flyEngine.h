/*! \file */

//! Fly3D engine class
/*! 
	This is the interface class with the engine. Its members represent all the engine-related options 
	available in Fly3D, and a few examples are: the current time, pointers to the current camera and
	player objects, the path where the engine was installed, the path for the resources, and much more.
	The methods provided by the flyEngine class vary from offline procedures like the load/save methods, 
	BSP building and static	lighting calculus functions to simulation-time methods like adding/removing
	objects from the scene, drawing the current frame viewed by the current active camera, recursing the 
	BSP for calculus of any kind, and much, much more.
*/
class FLY_ENGINE_API flyEngine
{
	private:
		int load_data();
		void save_bsp(const flyBspNode *n,FILE *fp) const;
		void load_bsp(flyBspNode **n,flyFile *fp);
		void find_leaf(flyBspNode *n);
		void sort_faces(int nfd,flyFace **fd);

	protected:
		void reset();
		void load_level();
		void compute_node_light();
		void compute_normals();

		int load_fpt(const char *file);
		int save_fpt(const char *file) const;

		int load_fmp(const char *file);
		int save_fmp(const char *file) const;

		int load_bsp(const char *file);
		int save_bsp(const char *file) const;

		int load_lightmaps(const char *file);
		int save_lightmaps(const char *file, int quality=-1) const;

		void alloc_pvs(int value);
		int load_pvs(const char *file);
		int save_pvs(const char *file) const;

		//! Smooth the path by dropping unnecessary vertices
		int smooth_path(const flyArray<flyVector> &path,flyArray<flyVector> &newpath);

	public:
		HWND hwnd;			//!< Window handle
		HINSTANCE hinst;	//!< Instance handle

		int	cur_step,			//!< current step (each call to each object step function in each frame has an unique id) 
			cur_step_base;		//!< current step base (first step id for current frame)  
		int cur_frame,			//!< current frame 
			cur_frame_base;		//!< current frame base (multiple renders are possible in different viewports) 
		int cur_bsprecurse;		//!< current BSP recursion (each call to each BSP recursion method from each object in each frame has an unique id) 
		int cur_time,			//!< current time in ms 
			cur_dt,				//!< current elapsed time from last frame in ms 
			force_dt;				
		float cur_time_float;	//!< current time in floating point format
		int start_time,			//!< global start time
			level_start_time,	//!< level start time
			frame_rate,			//!< current frame rate (frames per second)
			frame_count;		//!< total number of frames
		int fraglimit,			//!< multiplayer game frag limit
			timelimit,			//!< multiplayer game time limit
			restarttime;		//!< multiplayer game restart time
		int T0,					//!< last frame time
			T1;					//!< current frame time
		int objid;

		flyDllGroup dll;	//!< all plugin DLLs in current scene
		flyConsole con;		//!< console interface

		flyBspObject *player;		//!< current player
		flyBspObject *cam;			//!< current camera
		flyFrustum view;			//!< current view frustum
		double	cam_proj_mat[16],	//!< camera projection matrix
				cam_model_mat[16];	//!< camera model matrix
		int cam_viewport[4];		//!< camera viewport

		flyBspObject	*excludecollision;		//!< object to exclude from collision

		int nnodedraw;		//!< total number of drawn nodes
		int ntotfacedraw;	//!< total number of drawn faces
		int ntotelemdraw;	//!< total number of drawn elements
		
		flyArray<flyBspObject *> objdraw[FLY_MAX_OBJDRAWLAYERS];	//!< objects to draw in current render
		flyArray<flyFace *> facedraw;								//!< faces to draw in current render
		flyArray<flyFace *> facedrawtransp;							//!< transparent faces to draw in current render

		flyString flysdkpath;		//!< the sdk path (c:\flysdk\)
		flyString flysdkdatapath;   //!< the sdk data path (c:\flysdk\data\)
		flyString flysdkpluginpath;	//!< the sdk plugin path (c:\flysdk\plugin\)
		flyString flyfile;			//!< the .fly file (ship\ship.fly)
		flyString flyfilepath;		//!< the .fly path (c:\flysdk\data\ship\)
		flyString flyfilename;		//!< the .fly full file name (c:\flysdk\data\ship\ship.fly)
		flyString bspfile;			//!< the bsp file (ship)
		flyString status_msg;		//!< last message
		flyString console_command;	//!< command(s) to exectue
		flyString playername;		//!< player name for multiplayer games
		flyVector playercolor;		//!< player color on multiplayer games
		unsigned status_msg_time;	//!< time of the status string message
		
		flyBoundBox bbox;			//!< scene bounding box
		float	bboxdiag,			//!< scene bounding box diagonal
				geomdetail,			//!< level of geometric detail
				curveerr,			//!< level of curve error
				shadowdist,			//!< shadow distance from the object
				gravity;			//!< scene gravity
		flyVector	bboxcenter,		//!< scene bounding box center
					filter,			//!< screen filter color
					shadercolor,	//!< current shader color
					shadowcolor;	//!< shadow color
		int debug,					//!< debug mode (0: no debug, 1: debug)
			appid,					//!< frontend application id
			pvsoff,					//!< PVS mode (0: PVS culling, 1: no PVS)
			mpdelay,				//!< multiplayer message delay time
			antialias,				//!< antialiasing mode (0: no antialiasing, 1: antialiasing on)
			shadows,				//!< shadow flags (bit0: backface cull, bit1: suqare attenuation, bit2: N dot L attenuation, bit3: bspfaces shadows)
			collisionmode,			//!< bool to indicate current collision mode (1: pre-illumination, 2: pvs calculation)
			noinput,				//!< block input (0: normal, 1: no input)
			mute,					//!< mute flag (0: normal sound, 1: no sound)
			mouse,					//!< display mouse flag (0: no mouse display, 1: mouse normal draw)
			nodeonly,				//!< draw only the node containing the camera
			status,					//!< 2D drawing flag (0: no 2D drawing, 1: normal 2D drawing)
			crosshairpic,			//!< crosshair picture, leave blank for no crosshair
			crosshairsize,			//!< crosshair size in pixels
			loading,				//!< scene loading flag
			multitexture,			//!< multitexture mode (0: no multitexturing, 1: multitexturing on)
			fogmap,					//!< fogmap flag (0: no fogmap, 1: fogmap on)
			shadermask,				//!< bitfield for masking shader passes
			dynlight,				//!< dynamic lighting flag (0: off, 1: on)
			mastervolume,			//!< master volume for all sounds (0:maximum, <0:lower)
			picrender,				//!< flag to indicate rendeing to a picture or texture (0: normal render, 1: render to pictire, 2: render to texture)
			sse,					//!< flag to switch the use of Intel Pentium3 SSE intructions on/off
			vertprog,				//!< flag to switch the use of NVidia vertex  programs on/off
			octree,					//!< flag to switch the use of octrees for static and dynamic object
			bsppicflipy;			//!< flag to invert pictures from the bsp shader file (flip in y dir)
			
		//! hash linked list of active objects
		flyHashList active_objs;

		//! hash linked list of stock script objects
		flyHashList stock_objs;

		//! hash linked list of resource objects derived from flyBaseObject
		//! (flySound, flyBezierCurve, flyMesh, flyAnimatedMesh and flySkeletonMesh)
		flyHashList resource_objs;

		// ray intersection data
		flyBspObject *hitobj;	//!< current hit object
		flyMesh *hitmesh;		//!< current hit mesh
		int hitface,			//!< current hit face
			hitsubface,			//!< current hit subface
			hitshader;			//!< current hit face shader
		flyVector	hitip,		//!< last collision intersection point
					hitnormal;	//!< last collision hit normal
		float hitdist;			//!< last collision hit distance

		// vertex array for bsp faces
		flyVertex *vert;	//!< vertex array for BSP faces
		int nvert;			//!< total number of vertices in the 'vert' array

		// faces array for bsp
		flyFace *faces;	//!< face array for the BSP
		int nfaces;		//!< total number of faces in the 'faces' array

		// edges normal for bsp faces
		flyVector *en;	//!< edge array for BSP faces
		int nen;		//!< total number of edges in the 'en' array

		// array of shaders 
		flyArray<flyShader *> shaders;	//!< shaders dynamic array

		// the bsp tree
		flyBspNode *bsp;	//!< the BSP tree root

		// recurse bsp selected nodes
		flyArray<flyBspNode *> selnodes;	//!< array of selected nodes from the last BSP recursion
		
		// recurse bsp selected objects
		flyArray<flyBspObject *> selobjs;	//!< array of selected objects from the last BSP recursion

		// lightmaps
		flyArray<flyLightMap *> lm;			//!< individual light maps
		flyArray<flyLightMapPic *> lmpic;	//!< light map picture groups array
		int lmbase;							//!< offset into texture array for light map pictures
		flyArray<int> lmchanged;			//!< indices of all changed light maps

		// fogmaps
		flyArray<flyLightMap *> fm;			//!< individual fog maps
		flyArray<flyLightMapPic *> fmpic;	//!< fog map picture groups array
		int fmbase;							//!< offset into texture array for fog map pictures
		flyArray<int> fmchanged;			//!< indices of all changed fog maps

		flyArray<int> facechanged;			//!< indices of all changed triangle mesh faces

		// the pvs
		char *pvs;			//!< pvs bit array
		flyBspNode **leaf;	//!< BSP tree leaf array
		int nleaf,			//!< total number of leaves in the BSP tree
			pvssize,		//!< size of the PVS array in bytes
			pvsrowsize;		//!< size of each PVS row in bytes

		flyArray<flyInputMap> inputmaps; //!< array of input maps
		flyArray<flyBspObject *> poststeps; //!< array with post step needed objects

		int timedemo;		//!< flag indicating timedemo mode
		int timedemoframes;	//!< total timedemo frames already played
		FILE *demo_file_out;	//!< file dor demo recording
		flyFile *demo_file_in;	//!< file for demo playback
		void play_demo(const char *file);	//!< starts playing a demo file (.fdm)
		void record_demo(const char *file);	//!< starts recording a demo file (.fdm)

		//! Update scene for elapsed time from last frame
		int step();
		//! Update scene for elapsed time dt in miliseconds
		void step(int dt);
		//! Update scene method initial commands
		void step_init(int dt);
		//! Update scene method object commands
		void step_objects(int dt);
		//! Update scene method final commands
		void step_end(int dt);

		//! Draw all scene viewed from the current camera
		void draw_bsp();
		//! Draw the given faces
		void draw_faces(int nfd,flyFace **fd,flyVertex *v,int sort=1);
		//! Send drawing messages to all objects and draw everything in the current frame
		void draw_frame();
		// ! Same as draw_frame but draws into a flyPicture object
		int draw_frame_pic(flyPicture& p,flyBspObject *camera,float camangle,float aspect);
		// ! Same as draw_frame_pic but renders a sub-window
		int draw_frame_subpic(flyPicture& p,int x,int y,int sizex,int sizey,flyBspObject *camera,float camangle,float aspect);
		// ! Same as draw_frame but draws into a already created texture
		int draw_frame_tex(int tex,flyBspObject *camera,float camangle,float aspect);
		// ! Same as draw_frame_tex but renders a sub-window
		int draw_frame_subtex(int tex,int x,int y,int sizex,int sizey,flyBspObject *camera,float camangle,float aspect);
		
		//! Collision detection from p1 to p2, computes the closest collision
		int collision_bsp(const flyVector& p1,const flyVector& p2,int elemtype=0);
		//! Collision detection from p1 to p2, returns when finds the first collision
		int collision_test(const flyVector& p1,const flyVector& p2,int elemtype=0);

		//! Recurse the BSP selecting nodes and objects clipped by the sphere centered at p with radius rad
		void recurse_bsp(const flyVector& p,float rad,int elemtype,int pvsleaf=-1);
		//! Recurse the BSP selecting nodes and objects between p1 and p2
		void recurse_bsp(const flyVector& p1,const flyVector& p2,int elemtype,int pvsleaf=-1);
		//! Recurse the BSP selecting nodes and objects clipped by the volume defined by the array of points p
		void recurse_bsp(const flyVector *p,int np,int elemtype,int pvsleaf=-1);
		//! Recurse the BSP selecting nodes and objects clipped by the box defined by min and max
		void recurse_bsp_box(const flyVector& min,const flyVector& max,int elemtype,int pvsleaf=-1);

		//! Recurse the BSP sending messages to all selected objects
		void send_bsp_message(const flyVector& p,float rad,int msg,int param,void *data,int elemtype=0,int pvsleaf=-1);

		//! Recurse the BSP and return the node where the point p is located
		flyBspNode *find_node(const flyVector& p) const;

		//! Path finding
		int find_path(const flyVector &source,const flyVector &sourcedir,float sourceradius,const flyVector &dest,flyBezierCurve &curve);

		//! Add an object to the BSP tree
		void add_to_bsp(flyBspObject *obj);

		//! Open a .fly file
		int open_fly_file(const char *file);
		//! Close a .fly file
		void close_fly_file();
		//! Save to a .fly file
		int save_fly_file(const char *file);
	
		//! Print a string out on the console
		void console_printf(const char *fmt, ...);

		//! Load a picture to the texture cache or return its index if already loaded
		int get_picture(const char *file,int droplevel=0);
		//!	Load a shader file
		int load_shaders(const char *file);

		//! Get a mesh object by its filename (tga,jpg)
		flyMesh *get_model_object(const char *name);
		//! Get a sound object by its filename (wav)
		flySound *get_sound_object(const char *name);
		//! Get a bezier curve by its filename (bez)
		flyBezierCurve *get_bezier_curve(const char *name);
	
		//! Get a input map
		int get_input_map(const char *name);
		//! Checks the given input map for input
		int check_input_map(int i,int clicked=0);
		
		//! Set an object's parameter value
		int set_obj_param(const char *objname,const char *param,const char *value);
		//! Get an object's parameter value
		int get_obj_param(const char *objname,const char *param,flyString& value) const;
		//! Set a global parameter value
		int set_global_param(const char *name,const char *value);
		//! Get the value for a global parameter (scene, non-scene or from plugin)
		int get_global_param_desc0(int num,flyParamDesc *pd);
		//! Get the value of a non-scene dependant global parameter 
		int get_global_param_desc1(int i,flyParamDesc *pd);
		//! Get the value of a scene dependant global parameter 
		int get_global_param_desc2(int i,flyParamDesc *pd);

		//! Activate an object from the stock
		void activate(flyBspObject *d);
		//! Sets the current camera to the given object
		void set_camera(flyBspObject *d);
		//! Adds object to post processing list (executed after all objects step() and before any draw)
		virtual void add_post_step(flyBspObject *o);

		//! Join a multiplayer game
		int join_multiplayer(const char *addr,int num=0);
		//! Remove the player form the server
		void close_multiplayer();
		//! Check for multiplayer messages (should be called every frame in a multiplayer game)
		void check_multiplayer();
		//! Process one multiplayer message
		void process_multiplayer(int from,int size,const flyMPMsg *msg);
		
		//! Load the default texture (fonts and console)
		void load_default_tex(const char *path);
		//! Used to lock all input from game. 
		//! Call with no parameters (0) to lock and pass the return value to unlock. 
		//! Only one concurrent lock can exist so test return value for -1 (lock fail).
		int lock_input(int free=0);

		//! Load Fly3D configuration file (Fly3D.ini)
		int load_ini();
		//! Save Fly3D configuration file (Fly3D.ini)
		int save_ini();

		//! Default constructor
		flyEngine();
		//! Default destructor
		virtual ~flyEngine();
};
