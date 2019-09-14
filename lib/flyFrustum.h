/*! \file */

//! View frustum class
/*! 
	This class implements a view frustum, storing its vertices and planes. A frustum building method 
	and an aggressively fast bounding box clipping test for view culling are also provided. The class
	stores the frustum vertices and planes as member variables.
*/
class FLY_ENGINE_API flyFrustum
{
	public:
		flyVector verts[5];		//!< the frustum vertices
		flyVector planes[5];	//!< the frustum planes
		int bboxindx[8][3];		//!< the frustum bounding box's vertices index

	//! Default constructor
	flyFrustum() 
	{ 
		int i;
		for (i = 0; i<5; i++) 
		{
			verts[i]=0.0f;
			planes[i]=0.0f;
			bboxindx[i][0]=0;
			bboxindx[i][1]=0;
			bboxindx[i][2]=0;
		}
	}

	//! Copy-constructor
	flyFrustum(const flyFrustum& in) 
	{ 
		int i;
		for (i = 0; i<5; i++) 
		{
			verts[i]=in.verts[i];
			planes[i]=in.planes[i];
			bboxindx[i][0]=in.bboxindx[i][0];
			bboxindx[i][1]=in.bboxindx[i][1];
			bboxindx[i][2]=in.bboxindx[i][2];
		}
	}

	//! Default destructor
	virtual ~flyFrustum() 
	{ }

	//! Test the given bounding box against the frustum for clipping
	inline int clip_bbox(const flyBoundBox& bbox) const;
	//! Build the view frustum from position 'pos' and system given by (X,Y,Z)
	void build(const flyVector& pos,const flyVector& X,const flyVector& Y,const flyVector& Z,
		float camangle=flyRender::s_camangle,float aspect=flyRender::s_aspect,float farplane=flyRender::s_farplane);
	//! Draw the view frustum
	void draw() const;
};
