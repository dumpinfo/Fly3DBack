#include "../Fly3D.h"

int flyParticle::compute_collision(flyVector& p,flyVector& v)
{
	static flyVector p0,dir,d,v1,v2,reflectdir;

	float f1,f2,len;
	int ncol=-1;
	flyBspObject *last_hit_obj=0;
	flyMesh *last_hit_mesh=0;
	p0=pos;
	while(ncol<2)
	{
		dir=p-p0;
		len=dir.length();
		if (len<0.01f)
			break;
		dir/=len;
		if (ncol==-1)
		{
			p+=dir*0.1f;
			ncol++;
		}

		if (0==g_flyengine->collision_bsp(p0,p))
		{
			p0=p-dir*0.1f;
			break;
		}
		
		ncol++;

		last_hit_obj=g_flyengine->hitobj;
		last_hit_mesh=g_flyengine->hitmesh;
		p0=g_flyengine->hitip-dir*0.1f;

		reflectdir=dir+g_flyengine->hitnormal*(-2.0f*FLY_VECDOT(g_flyengine->hitnormal,dir));
		reflectdir.normalize();
		f1=FLY_VECDOT(g_flyengine->hitnormal,reflectdir);

		f2=(p-p0).length();
		d=reflectdir*f2;
		v1=g_flyengine->hitnormal*(f1*f2);
		v2=d-v1;
		p=p0+v1*bump+v2*friction;

		f2=v.length();
		d=reflectdir*f2;
		v1=g_flyengine->hitnormal*(f1*f2);
		v2=d-v1;
		v=v1*bump+v2*friction;
	}
	p=p0;
	g_flyengine->hitobj=last_hit_obj;
	g_flyengine->hitmesh=last_hit_mesh;
	return ncol;
}

int flyParticle::step(int dt)
{
	static flyVector p,v;
	float dtf=(float)dt;
	int move=1;
	
	p.x=pos.x+dtf*vel.x;
	p.y=pos.y+dtf*vel.y;
	p.z=pos.z+dtf*vel.z;
	v.x=vel.x+dtf*force.x;
	v.y=vel.y+dtf*force.y;
	v.z=vel.z+dtf*force.z;
	life-=dt;

	if (colflag&1)
	{
		move=compute_collision(p,v);
		if (move==-1)
			move=0;
		else
		{
			if (move&&(colflag&2))
				life=-1;
			move=1;
		}
	}

	pos=p;
	vel=v;

	return move;
}

