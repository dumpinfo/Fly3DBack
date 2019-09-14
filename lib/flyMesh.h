/*! \file */

//! Polygonal mesh class
/*!
	This class implements a 3D object polygonal mesh. Its faces may reference the local faces or 
	the global engine BSP faces. The class members are the number of faces and vertices of the mesh, 
	as well as pointers to the faces, vertices and edge normals. The methods supplied are for
	ray-intersection, normals computation, illumination, and much more.
*/
class FLY_ENGINE_API flyMesh : public flyBaseObject
{
	public:
		int	nf;					//!< number of faces
		int nv;					//!< numver of vertices
		int nen;				//!< number of edge normals

		flyFace **faces;		//!< the faces
		flyFace *localfaces;	//!< the local faces
		flyVertex *localvert;	//!< the local vertices
		flyVector *localen;		//!< the local edge normals

		flyVector pivotpos;		//!< pivot position
		flyBoundBox bbox;		//!< mesh bounding box

		flyVector color;		//!< color multiplier
		int lastdraw;			//!< last frame in ehich the mesh was drawn
		int nanims;				//!< number of animations
		int lastintesectedface;	//!< index of last face intersected (-1 for none)

	//! Default constructor
	flyMesh() :
		nf(0),nv(0),nen(0),
		faces(0),localfaces(0),
		localvert(0),localen(0),
		pivotpos(0),color(1),
		lastdraw(0),nanims(0),
		lastintesectedface(-1)
	{ type=FLY_TYPE_MESH; }

	//! Copy-constructor
	flyMesh(const flyMesh& in);

	//! Atribuition operator
	void operator=(const flyMesh& in);

	//! Default destructor
	virtual ~flyMesh()
	{ reset(); }

	//! Free the object
	virtual void reset();
	//! Draw all object's faces
	virtual void draw();
	//! Draw stencil shadow volume
	void draw_shadow_volume(const flyVector& lightdir);
	//! Draw raw geometry only
	void draw_plain();
	//! Load the mesh normals
	void compute_normals(int flag=7);
	//! Compute ray intersection with all object's faces
	int ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist) const;
	//! Return on first ray intersection
	int ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist);
	//! Dynamicaly illuminate the object's faces
	void illum_faces(const flyVector& ip,float d_max,const flyVector& c,int shadows) const;

	//! Save the mesh to a file
	virtual int save(const char *file) const ;
	//! Load the mesh from a file
	virtual int load(const char *file);

	//! Set the number of vertices
	void set_numverts(int nverts,int keep=0);
	//! Set the number of faces
	void set_numfaces(int nfaces,int local,int keep=0);
	//! Set the number of edge normals
	void set_numen(int numen,int keep=0);
};
