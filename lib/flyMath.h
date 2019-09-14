/*! \file */

#ifdef FLY_MATH_H
#else
#define FLY_MATH_H

#ifdef FLY_MATH_EXPORTS
#define FLY_MATH_API __declspec(dllexport)
#else
#define FLY_MATH_API __declspec(dllimport)
#endif

#include <memory.h>
#include <math.h>

#define INTEL_SIMD

FLY_MATH_API void *aligned_alloc(int n);
FLY_MATH_API void aligned_free(void *data);

//! random floating point number
#define FLY_FRAND (((rand()%10000)-5000)/5000.0f)
//! absolute random floating point number
#define FLY_FABSRAND ((rand()%10000)/10000.0f)

//! two vectors dot product
#define FLY_VECDOT(v1,v2) ((v1).x*(v2).x+(v1).y*(v2).y+(v1).z*(v2).z)

//! lower bound for overflow detection
#define FLY_SMALL 1.0e-4f
//! upper bound for overflow detection
#define FLY_BIG   1.0e+10f
//! underflow detection
#define FLY_ISZERO(a) ((a)>-FLY_SMALL && (a)<FLY_SMALL)

//! represents the value of 2*pi
#define FLY_2PI				6.28318530718f
//! represents the value of pi
#define FLY_PI				3.14159265359f
//! represents the value of pi/2
#define FLY_PI2				1.57079632679f
//! represents the value of pi/180
#define FLY_PIOVER180		1.74532925199433E-002f
//! represents the value of 180/pi
#define FLY_PIUNDER180		5.72957795130823E+001f
//! represents cos(45)
#define FLY_COS45			0.7071067811865475244f
//! represents the square root of 3
#define FLY_SQRT3			1.7320508075688772935f

//! returns the bitwise representation of a floating point number
#define FLY_FPBITS(fp)		(*(int *)&(fp))
//! returns the absolute value of a floating point number in bitwise form
#define FLY_FPABSBITS(fp)	(FLY_FPBITS(fp)&0x7FFFFFFF)
//! returns the signal bit of a floating point number
#define FLY_FPSIGNBIT(fp)	(FLY_FPBITS(fp)&0x80000000)
//! returns the absolute value of a floating point number
#define FLY_FPABS(fp)		(*((int *)&fp)=FLY_FPABSBITS(fp))
//! returns the value of 1.0f in bitwise form
#define FLY_FPONEBITS		0x3F800000

class FLY_MATH_API flyVector;		// 4 floats
class FLY_MATH_API flyQuaternion;	// 4 floats
class FLY_MATH_API flyMatrix;		// 16 floats
class FLY_MATH_API flyPlane;		// 5 floats
class FLY_MATH_API flyVertex;		// 8 floats, 1 int

//! Vector class compound of 4 floating point real numbers
/*!
	This is probably the most used class in the whole system. One can use flyVector
	with 3 or 4 floats, as in the first case the 4th value remains zero. One must
	address the components of flyVector by the letters x, y, z and w, or by indexes 
	0 to 3, and the operations of negation, normalization and cross product with another
	flyVector object are provided.
*/
class FLY_MATH_API flyVector
{
	public:
		float	x,	//!< the first component
				y,	//!< the second component
				z,	//!< the third component
				w;	//!< the fourth (of optional usage) component

	//! Default constructor
	flyVector() : x(0), y(0), z(0), w(1)
	{ }

	//! Constructor from a floating point value, sets x, y and z to this value, and w to 1.0f
	flyVector(float f) 
	{ x=y=z=f; w=1.0f; }

	//! Copy-constructor
	flyVector(const flyVector& v) : x(v.x), y(v.y), z(v.z), w(v.w) 
	{ }

	//! Constructor from 3 floating point numbers, sets x, y and z to their respective values, and w to 1.0f
	flyVector(float x0,float y0,float z0)
	{ x=x0; y=y0; z=z0; w=1;};

	//! Constructor from 4 floating point numbers
	flyVector(float x0,float y0,float z0,float w0)
	{ x=x0; y=y0; z=z0; w=w0; };

	//! Atribuition operator
	void operator=(const flyVector& in) 
	{ x = in.x; y = in.y; z = in.z; w = in.w; }

	//! Nullifying function, sets x, y and z to zero, and w to 1.0f
	inline void null(void)
	{ x=y=z=0; w=1; };

	//! Returns the length of the vector
	inline float length(void)
	{ return (float)sqrt(x*x+y*y+z*z); };

	//! Returns the square of the length
	inline float length2(void)
	{ return (float)x*x+y*y+z*z; };

	//!	Returns the distance to another flyVector
	inline float distto(const flyVector& v) const
	{ return (float)sqrt((v.x-x)*(v.x-x)+(v.y-y)*(v.y-y)+(v.z-z)*(v.z-z)); };

	//!	Returns the square of the distance to another flyVector
	inline float distto2(const flyVector& v) const
	{ return (float)(v.x-x)*(v.x-x)+(v.y-y)*(v.y-y)+(v.z-z)*(v.z-z); };

	//!	Set all the components to the given floating point value
	inline void vec(float f)
	{ x=y=z=w=f; };

	//!	Set components to the given floating point values, and set w=1.0f
	inline void vec(float x0,float y0,float z0)
	{ x=x0; y=y0; z=z0; w=1; };

	//!	Set components to the given floating point values
	inline void vec(float x0,float y0,float z0,float w0)
	{ x=x0; y=y0; z=z0; w=w0; };

	//!	Negate the first 3 components
	inline void negate(void)
	{ x=-x; y=-y; z=-z; };

	//!	Compute the cross-product of two given flyVectors
	inline void cross(const flyVector& v1, const flyVector& v2)
	{
		x=v1.y*v2.z-v1.z*v2.y;
		y=v1.z*v2.x-v1.x*v2.z;
		z=v1.x*v2.y-v1.y*v2.x;
	}

	//!	Normalize the vector
	inline void normalize(void)
	{
		float len=(float)sqrt(x*x+y*y+z*z);
		if (FLY_FPBITS(len)==0) return;
		len=1.0f/len;
		x*=len; y*=len; z*=len;
	}

	//!	Reference indexing operator
	inline float& operator[](int i) { return (&x)[i]; };
	//!	Indexing operator
	inline float operator[](int i) const { return (&x)[i]; }

	//!	Negates the vector (unary operator -)
	inline flyVector operator-() const { return flyVector(-x,-y,-z); }
};

//! Matrix class compound of 4x4 floating point real numbers
/*!
	This class implements a 4x4 matrix. 
	The matrix can represent any linear transformation and is used for rotation, translation, scaling, etc...
*/
class FLY_MATH_API flyMatrix
{
	public:
		float m[4][4]; //!< matrix elements

	//! Default constructor, sets the identity matrix
	flyMatrix() 
	{
		m[0][0]=m[1][1]=m[2][2]=m[3][3]=1.0f;
		m[0][1]=m[0][2]=m[0][3]=0.0f;
		m[1][0]=m[1][2]=m[1][3]=0.0f;
		m[2][0]=m[2][1]=m[2][3]=0.0f;
		m[3][0]=m[3][1]=m[3][2]=0.0f;
	}

	//! Copy-constructor
	flyMatrix(const flyMatrix& in) 
	{ 
		m[0][0]=in.m[0][0]; m[0][1]=in.m[0][1]; m[0][2]=in.m[0][2]; m[0][3]=in.m[0][3];
		m[1][0]=in.m[1][0]; m[1][1]=in.m[1][1]; m[1][2]=in.m[1][2]; m[1][3]=in.m[1][3];
		m[2][0]=in.m[2][0]; m[2][1]=in.m[2][1]; m[2][2]=in.m[2][2]; m[2][3]=in.m[2][3];
		m[3][0]=in.m[3][0]; m[3][1]=in.m[3][1]; m[3][2]=in.m[3][2]; m[3][3]=in.m[3][3];
	}

	//! Atribuition operator
	void operator=(const flyMatrix& in) 
	{ 
		m[0][0]=in.m[0][0]; m[0][1]=in.m[0][1]; m[0][2]=in.m[0][2]; m[0][3]=in.m[0][3];
		m[1][0]=in.m[1][0]; m[1][1]=in.m[1][1]; m[1][2]=in.m[1][2]; m[1][3]=in.m[1][3];
		m[2][0]=in.m[2][0]; m[2][1]=in.m[2][1]; m[2][2]=in.m[2][2]; m[2][3]=in.m[2][3];
		m[3][0]=in.m[3][0]; m[3][1]=in.m[3][1]; m[3][2]=in.m[3][2]; m[3][3]=in.m[3][3];
	}

	//! Nullify all elements
	inline void null(void)
	{
		m[0][0]=m[0][1]=m[0][2]=m[0][3]= 
		m[1][0]=m[1][1]=m[1][2]=m[1][3]= 
		m[2][0]=m[2][1]=m[2][2]=m[2][3]= 
		m[3][0]=m[3][1]=m[3][2]=m[3][3]=0.0f;
	}

	//! Load the identity matrix
	inline void load_identity(void)
	{
		m[0][0]=m[1][1]=m[2][2]=m[3][3]=1.0f;
		m[0][1]=m[0][2]=m[0][3]=0.0f;
		m[1][0]=m[1][2]=m[1][3]=0.0f;
		m[2][0]=m[2][1]=m[2][3]=0.0f;
		m[3][0]=m[3][1]=m[3][2]=0.0f;
	}

	//! Set the matrix as the rotation matrix of angle given by 'ang' around direction 'dir'
	void set_rotation( float ang, const flyVector& dir );
	//! Multiply the matrix by another with rotation given by 'ang' around direction 'dir'
	void rotate( float ang, const flyVector& dir );
	//! Interpolate between two rotation matrices using quaternions
	void lerp(const flyMatrix& m1, const flyMatrix& m2,float t);

	//! Multiplication operator
	inline flyMatrix operator*(const flyMatrix& m1) const
	{
	  flyMatrix m2;
	  int i,j;
	  for(i=0; i<4; i++)
		for(j=0; j<4; j++)
		{
			m2.m[i][j] = m[i][0]*m1.m[0][j] + m[i][1]*m1.m[1][j] +
				m[i][2]*m1.m[2][j] + m[i][3]*m1.m[3][j];
		}
	  return m2;
	}
};

//! Quaternion representation class
/*!
	This class implements a quaternion that represents a rotation of 'theta' degrees around an axis 'dir', 
	where: x=dir.x*sin(theta/2), y=dir.y*sin(theta/2), z=dir.z*sin(theta/2), w=cos(theta/2)
*/
class FLY_MATH_API flyQuaternion : public flyVector
{
public:
	//! Copy-constructor
	flyQuaternion(const flyQuaternion& in) : flyVector(in) 
	{ }

	//! Default constructor
	flyQuaternion() : flyVector() 
	{ }

	//! Construct the quaternion from a matrix
	flyQuaternion(const flyMatrix &mat);

	//! Construct the quaternion from the angle and axis
	flyQuaternion(float angle, const flyVector &axis)
	{
		float f=(float)sin(angle*FLY_PIOVER180*0.5f);
		x=axis.x*f;
		y=axis.y*f;
		z=axis.z*f;
		w=(float)cos(angle*FLY_PIOVER180*0.5f);
	}

	//! Normalize the quaternion
	void normalize();
	//! Converts the quaternion into a matrix
	void get_mat(flyMatrix &mat) const;
	//! Get the rotation angle and axis
	void get_rotate(float &angle, flyVector &axis) const;
	//! Interpolate two quaternions
	void lerp(const flyQuaternion& q1,const flyQuaternion& q2,float t);

	//! Multiplication between quaternions
	flyQuaternion operator *(const flyQuaternion& q);
	//! Addition between quaternions
	flyQuaternion operator +(const flyQuaternion& q);
};

//! Multiplies a vector by a floating point value
inline void operator*=(flyVector& v,float f)
{
  v.x*=f; v.y*=f; v.z*=f;
}
//! Divides a vector by a floating point value
inline void operator/=(flyVector& v,float f)
{
  v.x/=f; v.y/=f; v.z/=f;
}
//! Subtracts vector 'v2' from the original vector
inline void operator-=(flyVector& v1,const flyVector& v2)
{
  v1.x-=v2.x; v1.y-=v2.y; v1.z-=v2.z;
}
//! Adds the original vector with another
inline void operator+=(flyVector& v1, const flyVector& v2)
{
  v1.x+=v2.x; v1.y+=v2.y; v1.z+=v2.z;
}
//! Multiplies the original vector with another
inline void operator*=(flyVector& v1, const flyVector& v2)
{
  v1.x*=v2.x; v1.y*=v2.y; v1.z*=v2.z;
}
//! Divides the original vector by another
inline void operator/=(flyVector& v1, const flyVector& v2)
{
  v1.x/=v2.x; v1.y/=v2.y; v1.z/=v2.z;
}

//! Subtracts vector 'v2' from 'v1'
inline flyVector operator-(flyVector v1, const flyVector& v2)
{
   v1.x-=v2.x; v1.y-=v2.y; v1.z-=v2.z;
   return v1;
}

//! Adds two vectors
inline flyVector operator+(flyVector v1, const flyVector& v2)
{
   v1.x+=v2.x; v1.y+=v2.y; v1.z+=v2.z;
   return v1;
}
//! Multiplication between vectors
inline flyVector operator*(flyVector v1, const flyVector& v2)
{
   v1.x*=v2.x; v1.y*=v2.y; v1.z*=v2.z;
   return v1;
}
//! Divides vector 'v1' by vector 'v2'
inline flyVector operator/(flyVector v1, const flyVector& v2)
{
   v1.x/=v2.x; v1.y/=v2.y; v1.z/=v2.z;
   return v1;
}
//! Multiply a floating point value by a vector
inline flyVector operator*(float f,flyVector v)
{
   v.x*=f; v.y*=f; v.z*=f;
   return v;
}
//! Multiply a vector by a floating point value
inline flyVector operator*(flyVector v,float f)
{
   v.x*=f; v.y*=f; v.z*=f;
   return v;
}
//! Divide a vector by a floating point value
inline flyVector operator/(flyVector v,float f)
{
   v.x/=f; v.y/=f; v.z/=f;
   return v;
}

//! Multiplies a vector by a matrix
inline flyVector operator*(const flyVector& v,const flyMatrix& m)
{
	flyVector r;
	register float *f=(float *)&m;
	r.x = v.x*f[0] + v.y*f[4] + v.z*f[8] + v.w*f[12];
	r.y = v.x*f[1] + v.y*f[5] + v.z*f[9] + v.w*f[13];
	r.z = v.x*f[2] + v.y*f[6] + v.z*f[10] + v.w*f[14];
	r.w = v.x*f[3] + v.y*f[7] + v.z*f[11] + v.w*f[15];
	return r;
}
//! Multiplies a matrix by a vector 
inline flyVector operator*(const flyMatrix& m, const flyVector& v)
{
	flyVector r;
	register float *f=(float *)&m;
	r.x = v.x*f[0] + v.y*f[4] + v.z*f[8] + v.w*f[12];
	r.y = v.x*f[1] + v.y*f[5] + v.z*f[9] + v.w*f[13];
	r.z = v.x*f[2] + v.y*f[6] + v.z*f[10] + v.w*f[14];
	r.w = v.x*f[3] + v.y*f[7] + v.z*f[11] + v.w*f[15];
	return r;
}

//! Plane representation class
/*!
	This class implements a plane is 3D space.
	Classes like the bsp_node and face derive from a plane.
*/
class FLY_MATH_API flyPlane
{
	public:
		flyVector normal;	//!< Plane normal
		float d0;			//!< Perpendicular distance from the plane to the origin

	//! Default constructor
	flyPlane() : d0(0) 
	{ }
	
	//! Constructor from components
	flyPlane(const flyVector& n, float dist) : 
		normal(n),d0(dist)
	{ }

	//! Copy-constructor
	flyPlane(const flyPlane& in) : normal(in.normal), d0(in.d0) 
	{ }

	//! Atribuition operator
	void operator=(const flyPlane& in) 
	{ 
		normal = in.normal;
		d0 = in.d0;
	}

	//! Compute the perpendicular distance from a point to the plane
	inline float distance(const flyVector &v) const
		{ return FLY_VECDOT(normal,v)-d0; }

	//! Intersect a ray (ro,rd) with the plane and return intersection point (ip) and distance to intersection (dist)
	int ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist);
};

//! Vertex representation class
/*!
	This class holds all info about a vertex, including color, two sets of texture coordinates,
	position and normal.
*/
class FLY_MATH_API flyVertex : public flyVector
{
	public:
		flyVector texcoord;		//!< (x,y): texture coordinates; (z,w): lightmap texture coordinates
		flyVector normal;		//!< the vertex's normal
		unsigned color;			//!< the vertex's color
		unsigned color_static;		//!< the vertex's color from static lighting
#ifdef INTEL_SIMD
		unsigned dummy[2];
#endif
	
	//! Default constructor
	flyVertex() : flyVector(), color(-1), color_static(-1) 
	{ }

	//! Constructor from components
	flyVertex(flyVector& p,flyVector& tx,flyVector& n,unsigned c) :
		flyVector(p), texcoord(tx), normal(n), color(c), color_static(c)
	{ }
	
	//! Copy-constructor
	flyVertex(const flyVertex& in) : 
		flyVector(in), 
		texcoord(in.texcoord), 
		normal(in.normal), 
		color(in.color),
		color_static(in.color_static)
	{ }

	//! Atribuition operator
	void operator=(const flyVertex& in) 
	{
		flyVector::operator=(in);
		texcoord = in.texcoord;
		normal = in.normal;
		color = in.color;
		color_static = in.color_static;
	}
};

#endif