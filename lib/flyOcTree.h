/*! \file */

//! OcTree node class
/*! 
	This class implements an octree node used to speed up collision 
	detection with detail trinagle meshes or curved Bezier faces.
*/
class FLY_ENGINE_API flyOcTreeNode 
{
public:
	flyBoundBox bbox;			//!< Node bound box
	flyArray<int> faces;		//!< Node triangle faces
	flyOcTreeNode *nodes[8];	//!< Node childs
	
	//! Default constructor
	flyOcTreeNode();

	//! Default destructor
	virtual ~flyOcTreeNode();

	//! Copy constructor
	flyOcTreeNode(flyOcTreeNode& in);

	//! Split faces into child nodes if a subdivison is need (used on the octree build process)
	void build_node(int *triverts,flyVertex *verts);
};

//! OcTree class
/*! 
	This class implements an octree and is used to subdivide the space for 
	complex detail triangle meshes and Bezier curved faces speeding up collision
	detection with its triangles (used for point collision and bbox collision)
*/
class FLY_ENGINE_API flyOcTree 
{
public:
	flyOcTreeNode *root;		//!< Root node for octree
	flyFace *face;				//!< Face from where octree was created

	//! Default constructor
	flyOcTree();
	
	//! Default destructor
	virtual ~flyOcTree();
	
	//! Copy constructor
	flyOcTree(flyOcTree& in);
	
	//! Operator equal
	void operator=(flyOcTree& in);

	//! Free the tree data
	void reset();

	//! Builds the octree for the given triangle face or Bezier face
	void build_tree(flyFace *f);
	
	//! recurse octree and ray intersect test the triangles (just bool resoult with no intersection info) 
	int ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist) const;
	
	//! recurse octree and ray intersect the triangles for the closest collision 
	int ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist) const;

	// recurse octree and find all faces from nodes cliped by the given bbox
	void clip_bbox(const flyBoundBox& bbox,flyArray<int>& faces) const;

	//! draw the bbox as wireframe
	void draw();
};