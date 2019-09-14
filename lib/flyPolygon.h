/*! \file */

//! Polygon representation class
/*!
	This class implements a convex polygon with any number of vertices. Its members are an array of 
	vertices and an array of planes defined by the polygon edges. Methods are availabe for cutting 
	the polygon by custom generated planes and for ordering its vertices.
*/
class FLY_ENGINE_API flyPolygon : public flyPlane
{
	public:
		flyArray<flyVertex> verts;		//!< Dynamic array of polygon vertices
		flyArray<flyPlane> edgeplanes;	//!< Dynamic array of the planes defined by each edge

	//! Default constructor
	flyPolygon();

	//! Copy-constructor
	flyPolygon(const flyPolygon& in);

	//! Default destructor
	virtual ~flyPolygon();
	
	//! Atribuition operator
	void operator=(const flyPolygon& in);

	//! Properly order the polygon's vertices to front-face the given normal
	void order_verts();

	//! Builds a quad
	void build_quad(const flyVector& pos,const flyVector& x,const flyVector& y,const flyVector& z);
	
	//! Cut the polygon with the given plane
	void clip_plane(const flyPlane& plane);
	
	//! Cut the polygon edge
	void clip_edge(int i,const flyPlane& plane,flyVertex& v);
	
	//! Draw the polygon
	void draw();

	//! Intersect with another polygon, storing intersection in 'out' (if any)
	int intersect(flyPolygon &in, flyPolygon &out);

	//! Returns the area of the polygon
	float area();

	//! Computes a rendom point within the polygon
	int random_point(flyVector& point,float threshold=0.1f);
	
	//! Build a list of plane for each of the polygon edges
	void build_edgeplanes();

	//! Compute polygon normal
	void build_normal();

	//! Calculate the center of the polygon
	flyVector get_center() const;

	//! Add a vertex to the polygon
	void add(const flyVertex& v);
	void add(const flyVector& v);

	//! Remove all vertices from polygon (free all data)
	void clear();

private:
	int intersect_verts(const flyPolygon &in, flyPolygon &out);
	int intersect_edges(const flyPolygon &in, flyPolygon &out);
};
