#include "../Fly3D.h"

flyStaticMesh::flyStaticMesh() 
{ 
	type=FLY_TYPE_STATICMESH; 
	objmesh=new flyMesh; 
}

flyStaticMesh::~flyStaticMesh()
{ 
	delete objmesh; 
}

flyMesh *flyStaticMesh::ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist,int &facenum) const 
{
	if (objmesh)
	{
		facenum=objmesh->ray_intersect(ro,rd,ip,dist);
		if (facenum!=-1)
			return objmesh;
	}
	return 0;
}

int flyStaticMesh::ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist) const 
{
	if (objmesh)
		if (objmesh->ray_intersect_test(ro,rd,dist))
			return 1;
	return 0;
}

int flyStaticMesh::message(const flyVector& p,float rad,int msg,int param,void *data)
{
	if (msg==FLY_OBJMESSAGE_ILLUM)
		if (objmesh)
			objmesh->illum_faces(p,rad,*((flyVector *)data),param);
	return 0;
}
