/*! \file */

//! Static mesh class
/*!
	This class implements the group of geometry faces that define a convex volume, i.e. a BSP leaf node.
	The only member of the class is a flyMesh object representing the mesh. The methods available include
	one for retrievieng the mesh, two for ray-intersection and one for receiving messages.
*/
class FLY_ENGINE_API flyStaticMesh : public flyBspObject
{
	public:
		flyMesh *objmesh;	//!< the mesh object

	//! Return the mesh
	flyMesh *get_mesh() { return objmesh; }
	//! Ray intersection with leaf nodes faces
	flyMesh *ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist,int &facenum) const; 
	//! Ray intersection test with leaf nodes faces, returns after first intersection
	int ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist) const;
	//! Process the messages sent to the BSP leaf nodes faces
	int message(const flyVector& p,float rad,int msg,int param,void *data);

	//! Default constructor
	flyStaticMesh();

	//! Default destructor
	virtual ~flyStaticMesh();
};
