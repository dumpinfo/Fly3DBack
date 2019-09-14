/*! \file */

//! Vertex-animated mesh class
/*!
	This class implements key-based vertex-animated meshes that can be read from a .f3d file.
	Its members are: the number of key-frames and an array of vectors that represent the position 
	of every vertex of the model in every key-frame. The class also provides built-in blending 
	methods for multi-animation blending, as well as routines for reading and saving .f3d files,
	setting the current state of the model in an animation and blend between animations.
*/
class FLY_ENGINE_API flyAnimatedMesh : public flyMesh
{
public:
	int nkeys;												//!< total number of keys (sum of animkeys)
	int animkeys[FLY_MAX_MESHANIMS];						//!< number of keys in each animation
	int animkeyspos[FLY_MAX_MESHANIMS];						//!< start position of each key (accumulated animkeys)
	char animnames[FLY_MAX_MESHANIMS][FLY_MAX_MESHSTRLEN];	//!< animation names
	
	flyVector *key_verts; //!< vertices of every key in every animation (size: nkeys*nverts)

	//! Default constructor
	flyAnimatedMesh();

	//! Copy-constructor
	flyAnimatedMesh(const flyAnimatedMesh& in);

	//! Atribuition operator
	void operator=(const flyAnimatedMesh& in);

	//! Default destructor, calls reset
	virtual ~flyAnimatedMesh();
	
	//! Free all data
	void reset(void);

	//! Save the mesh to a file
	int save(const char *file) const;
	//! Load the mesh from a file
	int load(const char *file);

	//! Set the mesh to the desired animation and key
	void set_state(int anim, int key);
	//! Set the mesh to the desired animation, interpolating between keys in function of key_factor
	void set_state(int anim, float key_factor);
	//! Set the mesh to an intermediate state (blending) between two animations/keys
	void set_state(int anim1, float key_factor1, int anim2, float key_factor2, float blend);

	//! Blend the given key/animation with the current one
	void set_state_blendcur(int anim, int key, float blend);
	//! Blend the given animation (key given by key_factor) with the current one
	void set_state_blendcur(int anim, float key_factor, float blend);
};
