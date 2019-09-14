/*! \file */

//! Bone-animated mesh class
/*!
	This class implements skeletal animated meshes that can be read from a .k3d file. Some of its 
	members are: the number of keys in all animations, the names of the animations, the total number 
	of bones,the names of each bone, the root bone in the skeleton hierarchy and an array of vertex 
	weights. The class also features methods for building the mesh from the current animation and 
	skeleton state, blending between animations and skeleton states, drawing the skeleton for debug 
	purposes and loading and saving .k3d files.
*/
class FLY_ENGINE_API flySkeletonMesh : public flyMesh
{
public:
	int nkeys;												//!< number of animation keys
	int animkeys[FLY_MAX_MESHANIMS];						//!< animation keys array
	int animkeyspos[FLY_MAX_MESHANIMS];						//!< start position of each key (accumulated animkeys)
	char animnames[FLY_MAX_MESHANIMS][FLY_MAX_MESHSTRLEN];	//!< animation names
	char bonenames[FLY_MAX_MESHBONES][FLY_MAX_MESHSTRLEN];	//!< bone names
	
	int nbones;			//!< number of bones
	int *nvweights;		//!< list of numbers of vertex weights
	int *vindex;		//!< list of vertex indices
	int *parent;		//!< list of parent bones
	flyVector *vweight;	//!< vertex weights array
	flyMatrix *tm;		//!< list of all the bones matrices in all keys
	flyMatrix *m;		//!< list of the bones matrices in current skeleton
	int root;			//!< root bone

	//! Default constructor
	flySkeletonMesh() :
		nkeys(0),
		nbones(0),
		root(-1),
		nvweights(0),vindex(0),parent(0),
		vweight(0),tm(0),m(0)
	{ type=FLY_TYPE_SKELETONMESH; }

	//! Copy-constructor
	flySkeletonMesh(const flySkeletonMesh& in);

	//! Atribuition operator
	void operator=(const flySkeletonMesh& in);

	//! Default destructor
	virtual ~flySkeletonMesh()
	{ reset(); }

	//! Free all data
	void reset();

	//! Get the key factor in the animation given by 'anim'
	inline int get_key_factor(int anim,float& key_factor);

	//! Save the mesh to a file
	int save(const char *file) const;
	//! Load the mesh from a file
	int load(const char *file);

	//! Draw the skeleton
	void draw_skeleton() const;
	//! Get the index of a bone given its name
	int get_bone(const char *name) const;

	//! Set the skeleton to the desired animation and key
	void set_skeleton(int anim, int key);
	//! Set the skeleton to the desired animation, interpolating between keys in function of key_factor
	void set_skeleton(int anim, float key_factor);
	//! Set the skeleton to an intermediate state (blending) between two animations/keys
	void set_skeleton(int anim1, float key_factor1, int anim2, float key_factor2, float blend);

	//! Build the mesh from current skeleton
	void build_mesh();
};

