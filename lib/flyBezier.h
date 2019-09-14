/*! \file */

//! Any-dimension Bezier curve
/*! 
	This class implements a n-dimension Bezier curve. The curve and tangent can be 
	evaluated at any point, and the curve can be adaptively subdivided until the 
	curve error is less than a maximum error factor. The members in this class are: 
	the control points, the number of segments in the curve, the curve's dimension
	and the curve's pivot position. Some of its methods include: adding a control
	point to the curve, evaluate the curve and the tangent at a given point, loading
	a .bez file which contains data about a curve, adaptive subdivision of the 
	curve, and much more.
*/
class FLY_ENGINE_API flyBezierCurve : public flyBaseObject
{
protected:
	float distance(const float *p,const float *p1,const float *p2) const;
	void subdiv(float u1,float u2,const float *p1,const float *p2,float *points,int& npoints,float maxerror,int maxpoints) const;
	
public:
	float *p;			//!< curve control points
	int np,				//!< number of control points
		ns,				//!< number of bezier segments ((np-1)/3)
		nd;				//!< curve dimensions (2->2D, 3->3D, etc...)
	flyVector pivot;	//!< curve pivot position

	//! Default constructor
	flyBezierCurve();

	//! Copy-constructor
	flyBezierCurve(const flyBezierCurve& in);

	//! Destructor constructor
	virtual ~flyBezierCurve();

	//! Atribuition operator
	void operator=(const flyBezierCurve& in);
	
	//! Reset all allocated data
	void reset();
	//! Set the curve dimension from the given number (2: 2D, 3: 3D, ...)
	void set_dim(int ndim);
	//! Add a new point to the curve, the given array must have ndim floats
	void add_point(const float *f);
	//! Evaluate curve at point u (0..1), and store the resulting point in f (ndim floats)
	void evaluate(float u,float *f) const;
	//! Evaluate tangent of the curve at point u (0..1), and store the result in f (ndim floats)
	void evaluate_tangent(float u,float *f) const;
	
	//! Load a bezier curve from a .bez file
	int load_bez(const char *file);
	//! Adaptively subdivide the curve using maxerror threshold and store the resulting u coordinates in the given array
	int adaptative_subdiv(float maxerror,float *points,int maxpoints) const;
	//! Get the length of the curve
	float length() const;
};

//! Cubic or quadratic bezier patch
/*! 
	The patch can have any number of segments in u and v directions. 
	It is made of several connected cubic or quadratic Bezier patches 
	(nsu is the number of patches in the u dir and nsv in the v dir).

	For quadratic patches:
	nsu=(spu-2)/2
	nsv=(spv-2)/2

	For cubic patches:
	nsu=(spu-1)/2
	nsv=(spv-1)/2

	The surface is discretized based on the subdivision level. 
	The number of vertices in u and v dir are:

	nvertu=(1<<levelu)*nsu+1
	nvertv=(1<<levelv)*nsv+1

	At run-time, the surface car be draw at any level of detail up to its current selected level. 
	The set_detail method sets the desired level of detail.
	The numbrer of vertices  skipped (col/row draw loop increment) on each direction is: 

	nvertskip=(1<<nleveldrop)
*/
class FLY_ENGINE_API flyBezierPatch
{
	public:
		int mode;		//!< 2 for quadratic and 3 for cubic
		
		flyVertex *p;	//!< surface control points
		
		int np,	 //!< num points
			npu, //!< num points in u dir
			npv, //!< num points in v dir
			nsu, //!< num segments in u dir
			nsv; //!< num segments in u dir

		int levelu,			//!< subdivision level in u
			levelv;			//!< subdivision level in v
		int nvertu,			//!< surface num vertices in u
			nvertv;			//!< surface num vertices in v
		flyVertex *surf;	//!< discretized surface

		int numstrips;	//!< number of bezier curve strips
		int dropu,		//!< drop level in u
			dropv;		//!< drop level in v
		int dropu_inc,	//!< number of vertices to skip in u
			dropv_inc;	//!< number of vertices to skip in v

	//! Default constructor
	flyBezierPatch();
	
	//! Copy-constructor
	flyBezierPatch(const flyBezierPatch& in);
	
	//! Atribuition operator
	void operator=(const flyBezierPatch& in);

	//! Default destructor
	virtual ~flyBezierPatch();

	//! Reset all allocated data
	void reset();
	//! Build patch control points for a loft from 2 bezier curves (shape and path)
	void build_cp_loft(const flyBezierCurve *shape,const flyBezierCurve *path,float tileu,float tilev);
	//! Build patch control points from the given vertices
	void build_cp(int nu,int nv,const flyVertex *v);
	//! Evaluate curve data at point (u,v) ((0..1),(0..1)), and store the resulting vectors in dest (evaltype can specify point, normal or tangent)
	void evaluate(int evaltype,float u,float v,flyVector *dest) const;
	//! Build the highest resolution surface from the control points
	void build_surface();
	//! Draw the patch faces using dynamic LOD
	void draw(int drawmode) const;
	//! Automatically set the patch's drop level, based on its distance from camera
	void set_detail(const flyVector& pos,const flyVector& campos);
};

