/*! \file */

//! Generic object class
/*! 
	This class implements an object that exists in the simulation (and therefore is positioned 
	inside one or more BSP tree nodes). This is the parent class of all entities that participate 
	in the simulation. flyBspObject derives from 3 classes: flyParticle, from which it inherits
	physical entities like position, velocity, mass, bump and friction factors, and more; 
	flyLocalSystem, from which it inherits a coordinate system that represents the orientation of
	the object in the world; and flyBaseObject, from which it inherits name, type and other 
	identification parameters. The flyBspObject class also features some virtual functions that 
	should be overriden by the classes that inherit them; these methods implement initialization 
	of the object, state update	between the frames of the simulation, self-drawing, message 
	receiving, and much more.
*/
class FLY_ENGINE_API flyBspObject :	
							public flyBaseObject,
							public flyParticle, 
							public flyLocalSystem
{
	public:
		int id;		//!< object unique id
		
		flyBspObject *source;				//!< original object kept as a reference after cloning, NULL if from stock
		flyBoundBox bbox;					//!< bounding box used for clipping and colliding
		flyArray<flyBspNode *> clipnodes;	//!< BSP nodes clipping the object

		int latedraw;		//!< object's draw layer
		int lastdraw;		//!< last BSP draw id
		int lastbsprecurse;	//!< last BSP recursion id
		
		int collide;	//!< desired collison style (0: no collision, 1: bounding box collision, 2: full mesh collision)

		int active;		//!< active at startup
		flyVector rot;	//!< rotation at startup

	//! Default constructor
	flyBspObject();

	//! Copy-constructor
	flyBspObject(const flyBspObject& in);
	
	//! Atribuition operator
	void operator=(const flyBspObject& in);
		
	//! Default destructor
	virtual ~flyBspObject();
				
	//! Remove the object from the BSP tree
	void remove_from_bsp();

	//! Move the object dt miliseconds
	virtual int step(int dt)			
	{ return 0; };

	//! Get another instance similar to this
	virtual flyBspObject *clone() 
	{ return 0; };

	//! Return the object's polygonal mesh
	virtual flyMesh *get_mesh()
	{ return 0; };

	//! Initialisation method called for the stock and before each activation
	virtual void init()
	{ ; };

	//! Extra processing step called after all objects have alreayd done step().
	//! Needs calling flyEngine::post_process() passing the object for utilization.
	virtual void post_step()
	{ ; };

	//! FlyEditor's parameters description routine for this class
	int get_param_desc(int i,flyParamDesc *pd);

	//! FlyEditor's parameters description routine for any class derived from this one
	virtual int get_custom_param_desc(int i,flyParamDesc *pd)
	{ return 0; };

	//! Collide ray defined by ray origin (ro) and ray direction (rd) with the object 
	virtual flyMesh *ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist,int &facenum);
	//! Test for ray-intersection form a ray defined by ray origin (ro) and ray direction (rd) with the object 
	virtual int ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist);
	//! Collide using the object's bounding box
	int box_collision(flyVector& destpos,flyVector& destvel,int maxcol=3);

	//! Send a message of type msg to all objects defined by the sphere centered on p with radius rad
	virtual int message(const flyVector& p,float rad,int msg,int param,void *data)
	{ return 0; };

	//! Draw the object in its actual position
	virtual void draw();
	//! Draw the object's shadow
	virtual void draw_shadow() { };

	//! Load default parameters from a file
	void load_default_params(flyFile *file,const char *sec);
	//! Load parameters from a file
	void load_params(flyFile *file,const char *sec);
};
