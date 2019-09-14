/*! \file */

//! Axis-aligned bounding box class
/*!
	This class implements an axis-aligned bounding box (AABB), which every object in the scene must have.
	The bounding box is used in collision calculations, and for the engine's aggressive view culling. The
	AABB is defined by its minimum and maximum points, and this class provides methods for testing if a given 
	point is inside the box, testing if two AABBs intersect, calculating collision and ray-intersection.
*/
class FLY_ENGINE_API flyBoundBox
{
	public:
		flyVector	min,	//!< bounding box minimum point
					max;	//!< bounding box maximum point

	static int facevert[6][4];			//!< indices of the corresponding vertices for each face
	static int edgevert[12][2];			//!< indices of the corresponding vertices for each edge
	static int edgefaces[12][2];		//!< indices of the corresponding faces for each edge
	static flyVector vertnorm[8];		//!< normals of each vertex 
	static flyVector edgenorm[12];		//!< normals of each edge 
	static flyVector facenorm[6];		//!< normals of each face 
	static flyVector edgedir[12];		//!< edge directions normalized for each edge
	static flyVector edgedirth[12];		//!< edgedir divided by 100 (used as thresholds for edge collisions)

	//! Default constructor
	flyBoundBox() 
	{ 
		min = 0.0;
		max = 0.0;
	}

	//! Construct a bounding box given the minimum and maximum points
	flyBoundBox(const flyVector& min, const flyVector& max) 
	{ 
		flyBoundBox::min = min;
		flyBoundBox::max = max;
	}

	//! Copy-constructor
	flyBoundBox(const flyBoundBox& in)  
	{
		min = in.min;
		max = in.max;
	}

	//! Atribuition operator
	void operator=(const flyBoundBox& in) 
	{ 
		min = in.min;
		max = in.max;
	}

	//! Return the vertex corresponding to the given index
	inline flyVector get_vert(int ind) const 
	{
		switch(ind)
		{
		case 0: return min;
		case 1: return max;
		case 2: return flyVector(max.x,min.y,min.z);
		case 3: return flyVector(min.x,max.y,max.z);
		case 4: return flyVector(max.x,max.y,min.z);
		case 5: return flyVector(min.x,min.y,max.z);
		case 6: return flyVector(min.x,max.y,min.z);
		case 7: return flyVector(max.x,min.y,max.z);
		default: return flyVector(0,0,0);
		}
	}
	
	//! Return the distance of the plane corresponding to the given index (0=min[x], 1=min[y], 2=min[z], 3=max[x], 4=max[y], 5=max[z])
	inline float get_plane_dist(int ind) const 
	{
		return ind>2?max[ind-3]:min[ind];
	}

	//! Test for clipping between this and the bounding box whose minimum and maximum points are, respectively, bbmin and bbmax
	inline int clip_bbox(const flyVector& bbmin, const flyVector& bbmax) const 
	{
		if (max.x>=bbmin.x && min.x<=bbmax.x &&
			max.y>=bbmin.y && min.y<=bbmax.y &&
			max.z>=bbmin.z && min.z<=bbmax.z)
			return 1;
		return 0;
	}

	//! Test if point p is inside this bounding box
	inline int is_inside(const flyVector& p) const 
	{
		return	p.x>=min.x && p.x<=max.x &&
				p.y>=min.y && p.y<=max.y &&
				p.z>=min.z && p.z<=max.z;
	}

	//! Collide the bounding box moving in the direction dir with movement magnitude len and store its resulting position in p
	int collide(const flyVector& p,const flyVector& dir,float& len);

	//! Collide the bounding box moving in the direction dir with a static mesh
	int collide(flyMesh *mesh,const flyBoundBox& bbtemp,const flyVector& dir,float& len,flyVector& normal,flyVector& ip) const;
	//! Collide the bounding box moving in the direction dir with a dynamic mesh at position pos and orientation ls
	int collide(flyMesh *mesh,const flyVector& pos,const flyLocalSystem& ls,const flyBoundBox& bbtemp,const flyVector& dir,float& len,flyVector& normal,flyVector& ip) const;
	//! Collide the bounding box moving in the direction dir with movement magnitude len with another bounding box (bbox)
	int collide(const flyBoundBox& bbox,const flyVector& dir,float& len,flyVector& normal,flyVector& ip) const;
	
	//! Collide ray defined by ray origin (ro) and ray direction (rd) with the bounding box
	int ray_intersect(const flyVector& ro,const flyVector& rd,float& tnear,float& tfar) const;
	//! Collide edge (p1,p2) moving in direction dir with edge (p3,p4)
	int edge_collision(const flyVector& p1,const flyVector& p2,const flyVector& dir,const flyVector& p3,const flyVector& p4,float& dist,flyVector& ip) const;

	//! Reset all data 
	inline void reset() 
	{ 
		min.vec(FLY_BIG,FLY_BIG,FLY_BIG); 
		max.vec(-FLY_BIG,-FLY_BIG,-FLY_BIG); 
	}

	//! Add a point to the bounding box (expand its boundaries if necessary)
	void add_point(const flyVector &p);
	//! Draw the box
	void draw() const;
	//! Return a mesh corresponding to the bounding box faces
	flyMesh *get_mesh() const;
};
