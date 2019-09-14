/*! \file */

//! Single particle class
/*!
	This class implements a single, invisible, particle. Its members are mainly physical entities like
	position, velocity, mass and bump and friction factors. Among the methods are collision computing and 
	the step function, which moves the particle every frame.
*/
class FLY_ENGINE_API flyParticle
{
	public:
		flyVector pos,	//!< particle position
			vel,		//!< particle velocity
			force;		//!< particle force
		float mass,		//!< particle mass
			bump,		//!< particle bump factor (0..1)
			friction;	//!< particle friction factor (0..1)
		int life;		//!< particle life in miliseconds
		int colflag;	//!< particle collision flag (0: no collision, 1: collide, 3: collide and die)

	//! Computes particle collision
	int compute_collision(flyVector& p,flyVector& v);

	//! Move the particle 'dt' miliseconds
	int step(int dt);

	//! Default constructor
	flyParticle() : 
		pos(0,0,0),vel(0,0,0),force(0,0,0), 
		mass(1),bump(1),friction(1),
		life(0),colflag(0)
	{ }

	//! Copy-constructor
	flyParticle(const flyParticle& in) :
		pos(in.pos),vel(in.vel),force(in.force), 
		mass(in.mass),bump(in.bump),friction(in.friction),
		life(in.life),colflag(in.colflag)
	{ }

	//! Atribuition operator
	void operator=(const flyParticle& in) 
	{
		pos=in.pos;
		vel=in.vel;
		force=in.force;
		mass=in.mass;
		bump=in.bump;
		friction=in.friction;
		life=in.life;
		colflag=in.colflag;
	}

	//! Default destructor
	virtual ~flyParticle() { };
};
