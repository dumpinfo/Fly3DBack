/*! \file */

//! Local coordinates system class
/*!
	This class implements a local coordinates system defined by three perpendicular vectors (the base axis).
	The class members are the unitary vectors X, Y and Z that define the system, the rotation matrix that
	maps the system to world coordinates and the transpose of this matrix (stored for efficiency. The methods 
	available include updating the rotation matrices, rotating the system and aligning the Z vector with a 
	given direction.
*/
class FLY_ENGINE_API flyLocalSystem
{
	public:
		flyVector	X,		//!< x vector in the base axis
					Y,		//!< y vector in the base axis
					Z;		//!< z vector in the base axis
		flyMatrix	mat,	//!< rotation matrix
					mat_t;	//!< transpose of the rotation matrix

	//! Update the rotation matrices
	void update_mat();
	//! Rotate the local system axis by 'rot'
	void rotate(const flyVector& rot);
	//! Rotate the local system axis of angle 'ang' around vector 'v'
	void rotate(float ang,const flyVector& v);
	//! Rotate the local system axis from 'v' to 'u', in the plane defined by 'v' and 'u', at a maximum angle of 'maxang' degrees
	void rotate(const flyVector &v, const flyVector &u, float maxang=360);
	//! Align the z axis of the local system with the given vector
	void align_z(const flyVector& z);
	
	//! Default constructor
	flyLocalSystem() : 
		X(1,0,0), Y(0,1,0), Z(0,0,1) 
	{ }

	//! Copy-constructor
	flyLocalSystem(const flyLocalSystem& in) : 
		X(in.X), Y(in.Y), Z(in.Z), mat(in.mat), mat_t(in.mat_t) 
	{ }

	//! Atribuition operator
	void operator=(const flyLocalSystem& in) 
	{ 
		X = in.X;
		Y = in.Y;
		Z = in.Z;
		mat = in.mat;
		mat_t = in.mat_t;
	}

	//! Default destructor
	virtual ~flyLocalSystem() 
	{ }
};
