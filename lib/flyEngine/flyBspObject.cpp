#include "../Fly3D.h"

flyBspObject::flyBspObject() :
	id(0),source(0),
	latedraw(0),lastdraw(0),lastbsprecurse(0),
	collide(0),active(0),rot(0,0,0)
{ 
	type=FLY_TYPE_NONE; 
}

flyBspObject::~flyBspObject()
{ 
	if (clipnodes.num) 
		remove_from_bsp(); 
}

flyBspObject::flyBspObject(const flyBspObject& in) :
	flyBaseObject(in),
	flyParticle(in),
	flyLocalSystem(in),
	id(in.id),source((flyBspObject *)&in),
	bbox(in.bbox),latedraw(in.latedraw),lastdraw(in.lastdraw),
	lastbsprecurse(in.lastbsprecurse),collide(in.collide),
	active(in.active),rot(in.rot)
{
	for( int i=0;i<in.clipnodes.num;i++ )
		clipnodes.add(in.clipnodes[i]);
}

void flyBspObject::operator=(const flyBspObject& in)
{
	flyBaseObject::operator=(in);
	flyParticle::operator=(in);
	flyLocalSystem::operator=(in);
	id=in.id;
	source=in.source;
	bbox=in.bbox;
	clipnodes.clear();
	latedraw=in.latedraw;
	lastdraw=in.lastdraw;
	lastbsprecurse=in.lastbsprecurse;
	collide=in.collide;
	active=in.active;
	rot=in.rot;
	for( int i=0;i<in.clipnodes.num;i++ )
		clipnodes.add(in.clipnodes[i]);
}

int flyBspObject::box_collision(flyVector& destpos,flyVector& destvel,int maxcol)
{
	static flyVector p,dir,v1,v2,reflectdir;
	static float len,f1,f2,colnearest;
	int ncol=0;

	p=pos;

	while(ncol<maxcol)
	{
		// compute move direction and length
		dir=destpos-p;
		len=dir.length();
		if (len<0.01f)
			break;
		dir/=len;

		// find closest collision
		colnearest=len+0.1f;
		if(!bbox.collide(p,dir,colnearest))
		{
			// if no collision, move to desired position 
			// less error threshold
			p=destpos;
			break;
		}

		// compute new position moving to the closest collision
		// less error thershold
		p += dir*(colnearest+0.1f/FLY_VECDOT(g_flyengine->hitnormal,dir));
		ncol++;

		// compute reflect direction
		reflectdir=dir + g_flyengine->hitnormal*(-2.0f*FLY_VECDOT(g_flyengine->hitnormal,dir));
		reflectdir.normalize();
		
		// compute new destination position
		f1=FLY_VECDOT(g_flyengine->hitnormal,reflectdir);
		f2=(destpos-p).length();
		v1=g_flyengine->hitnormal*(f1*f2);
		v2=reflectdir*f2-v1;
		destpos=p+v1*bump+v2*friction;

		f2=destvel.length();
		if (FLY_FPBITS(f2)!=0)
		{
			// compute reflect direction
			destvel/=f2;
			reflectdir=destvel + g_flyengine->hitnormal*(-2.0f*FLY_VECDOT(g_flyengine->hitnormal,destvel));
			reflectdir.normalize();

			// compute new velocity 
			f1=FLY_VECDOT(g_flyengine->hitnormal,reflectdir);
			v1=g_flyengine->hitnormal*(f1*f2);
			v2=reflectdir*f2-v1;
			destvel=v1*bump+v2*friction;
		}
	}
	
	destpos=p;
	return ncol;
}

void flyBspObject::draw()
{
	flyMesh *objmesh=get_mesh();
	if (objmesh)
	{
		glPushMatrix();
		glTranslatef(pos.x,pos.y,pos.z);
		glMultMatrixf((float *)&mat);
		objmesh->draw();
		glPopMatrix();
	}
}

flyMesh *flyBspObject::ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist,int &facenum)
{ 
	flyMesh *objmesh=get_mesh();

	if (objmesh) 
	{
		static float d1,d2;
		flyVector ro_local=(ro-pos)*mat_t;
		flyVector rd_local=rd*mat_t;
		if (objmesh->bbox.ray_intersect(ro_local,rd_local,d1,d2)!=-1)
		{
		facenum=objmesh->ray_intersect(ro_local,rd_local,ip,dist);
		if (facenum>-1)
			{
			ip=ip*mat+pos;
			return objmesh;
			}
		}
	}
	return 0;
}

int flyBspObject::ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist)
{ 
	flyMesh *objmesh=get_mesh();

	if (objmesh) 
	{
		static float d1,d2;
		flyVector ro_local=(ro-pos)*mat_t;
		flyVector rd_local=rd*mat_t;
		if (objmesh->bbox.ray_intersect(ro_local,rd_local,d1,d2)!=-1)
		if (objmesh->ray_intersect_test(ro_local,rd_local,dist))
			return 1;
	}
	return 0;
}

void flyBspObject::remove_from_bsp()
{
	int i,j;
	for( i=0;i<clipnodes.num;i++ )
	{
		for( j=0;j<clipnodes[i]->elem.num;j++ )
			if (clipnodes[i]->elem[j]==this)
				break;
		if (j!=clipnodes[i]->elem.num)
			clipnodes[i]->elem.remove(j);
	}
	clipnodes.clear();
}

void flyBspObject::load_params(flyFile *file,const char *sec)
{
	int i,n=get_param_desc(0,0);
	flyParamDesc pd;
	flyString ret;

	for( i=0;i<n;i++ )
	{
		get_param_desc(i,&pd);
		ret="";
		if (file->get_profile_string(sec,pd.name,ret))
			pd.set_string(ret);
	}
}

void flyBspObject::load_default_params(flyFile *file,const char *sec)
{
	int i,n=FLY_NUMDEFAULTPARAM;
	flyParamDesc pd;
	flyString ret;

	for( i=0;i<n;i++ )
	{
		get_param_desc(i,&pd);
		ret="";
		if (file->get_profile_string(sec,pd.name,ret))
			pd.set_string(ret);
	}
}

int flyBspObject::get_param_desc(int i,flyParamDesc *pd)
{
	if (pd==0)
		return FLY_NUMDEFAULTPARAM+get_custom_param_desc(0,0);
	else
	if (i>=FLY_NUMDEFAULTPARAM)
		get_custom_param_desc(i-FLY_NUMDEFAULTPARAM,pd);
	else
	switch(i)
	{
		case 0:
			pd->type='v';
			pd->data=&pos;
			pd->name="pos";
			break;
		case 1:
			pd->type='v';
			pd->data=&rot;
			pd->name="rot";
			break;
		case 2:
			pd->type='v';
			pd->data=&vel;
			pd->name="vel";
			break;
		case 3:
			pd->type='v';
			pd->data=&force;
			pd->name="force";
			break;
		case 4:
			pd->type='i';
			pd->data=&life;
			pd->name="life";
			break;
		case 5:
			pd->type='i';
			pd->data=&colflag;
			pd->name="colflag";
			break;
		case 6:
			pd->type='i';
			pd->data=&collide;
			pd->name="collide";
			break;
		case 7:
			pd->type='f';
			pd->data=&mass;
			pd->name="mass";
			break;
		case 8:
			pd->type='f';
			pd->data=&bump;
			pd->name="bump";
			break;
		case 9:
			pd->type='f';
			pd->data=&friction;
			pd->name="friction";
			break;
		case 10:
			pd->type='i';
			pd->data=&active;
			pd->name="active";
			break;
		case 11:
			pd->type='i';
			pd->data=&latedraw;
			pd->name="latedraw";
			break;
	}
	return FLY_NUMDEFAULTPARAM;
}


