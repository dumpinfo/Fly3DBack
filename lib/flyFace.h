/*! \file */

//! General face class
/*!
	This class implements a face of any type supported by the engine, which are:

	Large convex polygons
	Bi-quadratic Bezier patches
	Triangular surfaces
	Triangular strips
	Triangular fans

	Its members are: the face type (an integer representing one of the types enumerated above),
	the number of vertices of the face, a pointer ti the first vertex in the engine's vertex 
	array, and many other members specific to each type of face. Among the methods provided by
	this class are: computing the normal of the face and computing ray-intersection with the face.
*/
class FLY_ENGINE_API flyFace : public flyPlane
{
	public:
		int facetype;	//!< Face type
		int flag;		//!< Face flags (0:detail, 1:structure)

		int nvert;			//!< number of vertices
		int vertindx;		//!< index of the first vertex
		flyVertex *vert;	//!< pointer to the first vertex
		flyVector *en;		//!< edge normals pointer

		int ntriface;	//!< Traingular surfaces: number of triangular faces
		int ntrivert;	//!< Traingular surfaces: number of vertices
		int *trivert;	//!< Traingular surfaces: pointer to the first of the triangular vertices

		int patch_npu;			//!< Patches: number of points in u
		int patch_npv;			//!< Patches: number of points in v
		flyBezierPatch *patch;	//!< Patches: pointer to the patch class
		flyVector pivot;		//!< Patches: the patch's pivot position

		int sh;	//!< applied shader index
		int lm;	//!< lightmap index
		
		int indx;			//!< index into faces array
		int sortkey;		//!< draw sort key
		int lastbsprecurse;	//!< last BSP recursion id
		int lastupdate;		//!< last vertex illumination step id

		flyBoundBox bbox;	//!< the face's bounding box
		flyOcTree *octree;	//!< OcTree with triangle faces for collision detection

	//!< Default constructor
	flyFace();

	//!< Copy-constructor
	flyFace(const flyFace& in);

	//! Default destructor
	virtual ~flyFace();

	//!< Atribuition operator
	void operator=(const flyFace& in);

	//! Compute face and face edges normals
	void compute_normals(int flag);
	//! Compute ray intersection with the face
	int ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist) const;
	//! Test for ray intersection with the face (faster but only a bool result with no intersection info)
	int ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist) const;
	
	//! Compute ray intersection with the triangle mesh faces specified in the facesindx list (used by octree ray_intersect)
	int ray_intersect_tri(int *facesindx,int facesnum,const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist) const;
	//! Test for ray intersection with the triangle mesh faces specified in the facesindx list 
	//! (used by octree ray_intersect_test, faster but only a bool result with no intersection info)
	int ray_intersect_tri_test(int *facesindx,int facesnum,const flyVector& ro,const flyVector& rd,float dist) const;

	//! Convert point p in face u,v coordinates
	void inverse_map(const flyVector& p,float& u,float& v) const;
	//! Convert face u,v coordinates to point p
	void forward_map(float u,float v,flyVector& p) const;
	//! Allocate vertices for a triangle mesh face
	void set_ntrivert(int n3v);
	// Computes the face area
	float area();
};
