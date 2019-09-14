#include "../Fly3D.h"

flyFace::flyFace() :
	facetype(0),flag(0),nvert(0),
	vertindx(-1),vert(0),en(0),
	ntriface(0),ntrivert(0),trivert(0),
	patch_npu(0),patch_npv(0),patch(0),
	pivot(0),sh(-1),lm(-1),
	indx(-1),sortkey(0),lastbsprecurse(0),
	lastupdate(0),octree(0)
{
}

flyFace::flyFace(const flyFace& in) :
	facetype(in.facetype),flag(in.flag),nvert(in.nvert),
	vertindx(in.vertindx),vert(in.vert),en(in.en),
	ntriface(in.ntriface),ntrivert(in.ntrivert),
	patch_npu(in.patch_npu),patch_npv(in.patch_npv),
	pivot(in.pivot),sh(in.sh),lm(in.lm),
	indx(in.indx),sortkey(in.sortkey),
	lastbsprecurse(in.lastbsprecurse),
	bbox(in.bbox),octree(in.octree)
{ 
	if (in.trivert)
	{
		trivert=new int[ntrivert];
		for( int i=0;i<ntrivert;i++ )
			trivert[i]=in.trivert[i];
	}
	if (in.patch)
		patch=new flyBezierPatch(*in.patch);
	else
		patch=0;
}

flyFace::~flyFace()
{
	delete[] trivert;
	delete patch;
}

void flyFace::operator=(const flyFace& in)
{
	facetype=in.facetype;
	flag=in.flag;
	nvert=in.nvert;
	vertindx=in.vertindx;
	vert=in.vert;
	en=in.en;
	ntriface=in.ntriface;
	ntrivert=in.ntrivert;
	patch_npu=in.patch_npu;
	patch_npv=in.patch_npv;
	pivot=in.pivot;
	sh=in.sh;
	lm=in.lm;
	indx=in.indx;
	sortkey=in.sortkey;
	lastbsprecurse=in.lastbsprecurse;
	lastupdate=in.lastupdate;
	octree=in.octree;
	
	if (in.trivert)
	{
		trivert=new int[ntrivert];
		for( int i=0;i<ntrivert;i++ )
			trivert[i]=in.trivert[i];
	}
	if (in.patch)
		patch=new flyBezierPatch(*in.patch);
	else
		patch=0;
}

void flyFace::set_ntrivert(int n3v)
{
	ntrivert=n3v;
	trivert=new int[n3v];
}

void flyFace::compute_normals(int flag)
{
	flyVector v1,v2;
	int i,j;

	if (facetype==FLY_FACETYPE_LARGE_POLYGON)
	{
		if (flag&FLY_MESH_FACENORM)
		{
			v1=vert[1]-vert[0];
			v2=vert[2]-vert[0];
			normal.cross(v1,v2);
			normal.normalize();
			d0=FLY_VECDOT(normal,*vert);

			if (en)
			for( i=0;i<nvert;i++ )
			{
				en[i].cross(
					vert[(i+1)%nvert]-vert[i],
					normal);
				en[i].normalize();
			}
		}
		if (flag&FLY_MESH_VERTNORM)
			for( i=0;i<nvert;i++ )
				vert[i].normal=normal;
		
		if (flag&FLY_MESH_BBOX)
		{
			if (nvert==0)
			{
				bbox.min.null();
				bbox.max.null();
			}
			else
			{
				bbox.reset();
				for( i=0;i<nvert;i++ )
					bbox.add_point(vert[i]);
			}
		}
	}
	else 
	if (facetype==FLY_FACETYPE_BEZIER_PATCH ||
		facetype==FLY_FACETYPE_TRIANGLE_MESH)
	{
		if (flag&FLY_MESH_FACENORM)
		{
			j=0;
			for( i=0;i<ntrivert;i+=3,j+=4 )
				{
				v1=vert[trivert[i+1]]-vert[trivert[i]];
				v2=vert[trivert[i+2]]-vert[trivert[i]];
				en[j].cross(v1,v2);
				en[j].normalize();
				en[j].w=FLY_VECDOT(en[j],vert[trivert[i]]);
				
				en[j+1].cross(
						vert[trivert[i+1]]-vert[trivert[i]],
						en[j]);
				en[j+1].normalize();
				en[j+2].cross(
						vert[trivert[i+2]]-vert[trivert[i+1]],
						en[j]);
				en[j+2].normalize();
				en[j+3].cross(
						vert[trivert[i]]-vert[trivert[i+2]],
						en[j]);
				en[j+3].normalize();

				if (flag&FLY_MESH_VERTNORM)
					{
					vert[trivert[i]].normal+=en[j];
					vert[trivert[i+1]].normal+=en[j];
					vert[trivert[i+2]].normal+=en[j];
					}
				}
			if (flag&FLY_MESH_VERTNORM)
			if (facetype==FLY_FACETYPE_BEZIER_PATCH)
				for( i=0,j=patch->nvertu*patch->nvertv;i<j;i++ )
					vert[i].normal.normalize();
			else
				for( i=0;i<nvert;i++ )
					vert[i+vertindx].normal.normalize();
			normal.vec(0,0,0);
		}

		if (flag&FLY_MESH_BBOX)
		{
			if (nvert==0)
			{
				bbox.min.null();
				bbox.max.null();
			}
			else
			{
				bbox.reset();
				if (facetype==FLY_FACETYPE_BEZIER_PATCH)
					if (patch)
						for( i=0,j=patch->nvertu*patch->nvertv;i<j;i++ )
							bbox.add_point(vert[i]);
					else
					for( i=0;i<nvert;i++ )
						bbox.add_point(vert[i]);
				else
					for( i=0;i<nvert;i++ )
						bbox.add_point(vert[i+vertindx]);
			}
		}
	}
}

int flyFace::ray_intersect_tri_test(int *facesindx,int facesnum,const flyVector& ro,const flyVector& rd,float dist) const
{
	flyVector v;
	float d,x;
	int i,j,k,f;
	for( i=0;i<facesnum;i++ )
	{
		j=facesindx[i]*4;

		x=FLY_VECDOT(en[j],rd);
		if (flag && FLY_FPSIGNBIT(x)==0)
			continue;
		d=(en[j].w - FLY_VECDOT(en[j],ro))/x;
		if (FLY_FPSIGNBIT(d) || d>=dist)
			continue;

		v=ro+rd*d;
		f=facesindx[i]*3;
		k=trivert[f];
		if ((v.x-vert[k].x)*en[j+1].x+
			(v.y-vert[k].y)*en[j+1].y+
			(v.z-vert[k].z)*en[j+1].z>0)
			continue;
		k=trivert[f+1];
		if ((v.x-vert[k].x)*en[j+2].x+
			(v.y-vert[k].y)*en[j+2].y+
			(v.z-vert[k].z)*en[j+2].z>0)
			continue;
		k=trivert[f+2];
		if ((v.x-vert[k].x)*en[j+3].x+
			(v.y-vert[k].y)*en[j+3].y+
			(v.z-vert[k].z)*en[j+3].z>0)
			continue;

		return 1;
	}
	return 0;
}

int flyFace::ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist) const
{
	int flag=g_flyengine->shaders[sh]->flags;
	
	if ((flag&FLY_SHADER_NOCOLLIDE) ||
		((flag&FLY_SHADER_NOSHADOWS)&&g_flyengine->collisionmode==1)||
		((flag&FLY_SHADER_TRANSPARENT)&&g_flyengine->collisionmode==2))
		return 0;
	flag=!(flag&FLY_SHADER_NOCULL);

	if (facetype==FLY_FACETYPE_LARGE_POLYGON)
	{
		float x=FLY_VECDOT(normal,rd);
		if (flag && FLY_FPSIGNBIT(x)==0)
			return 0;
		float d=(d0-FLY_VECDOT(normal,ro))/x;
		if (FLY_FPSIGNBIT(d) || d>=dist)
			return 0;

		flyVector v=ro+rd*d;

		for( int i=0;i<nvert;i++ )
			if ((v.x-vert[i].x)*en[i].x+
				(v.y-vert[i].y)*en[i].y+
				(v.z-vert[i].z)*en[i].z>0)
				return 0;

		return 1;
	}
	else
	{
		if (octree&&g_flyengine->octree)
		{
			if (octree->ray_intersect_test(ro,rd,dist))
				return 1;
		}
		else
		{
			flyVector v;
			float d,x;
			int i=0,j=0;
			for( ;i<ntrivert;i+=3,j+=4 )
			{
				x=FLY_VECDOT(en[j],rd);
				if (flag && FLY_FPSIGNBIT(x)==0)
					continue;
				d=(en[j].w - FLY_VECDOT(en[j],ro))/x;
				if (FLY_FPSIGNBIT(d) || d>=dist)
					continue;

				v=ro+rd*d;

				if ((v.x-vert[trivert[i]].x)*en[j+1].x+
					(v.y-vert[trivert[i]].y)*en[j+1].y+
					(v.z-vert[trivert[i]].z)*en[j+1].z>0)
					continue;
				if ((v.x-vert[trivert[i+1]].x)*en[j+2].x+
					(v.y-vert[trivert[i+1]].y)*en[j+2].y+
					(v.z-vert[trivert[i+1]].z)*en[j+2].z>0)
					continue;
				if ((v.x-vert[trivert[i+2]].x)*en[j+3].x+
					(v.y-vert[trivert[i+2]].y)*en[j+3].y+
					(v.z-vert[trivert[i+2]].z)*en[j+3].z>0)
					continue;

				return 1;
			}
			return 0;
		}
	}
	return 0;
}

int flyFace::ray_intersect_tri(int *facesindx,int facesnum,const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist) const 
{
	int i,j,k,f,r=-1;
	flyVector v;
	float d,x;
	dist=FLY_BIG;
	for( i=0;i<facesnum;i++ )
	{
		j=facesindx[i]*4;

		x=FLY_VECDOT(en[j],rd);
		if (flag && FLY_FPSIGNBIT(x)==0)
			continue;
		d=(en[j].w - FLY_VECDOT(en[j],ro))/x;
		if (FLY_FPSIGNBIT(d) || d>=dist)
			continue;

		v=ro+rd*d;

		f=facesindx[i]*3;
		k=trivert[f];
		if ((v.x-vert[k].x)*en[j+1].x+
			(v.y-vert[k].y)*en[j+1].y+
			(v.z-vert[k].z)*en[j+1].z>0)
			continue;
		k=trivert[f+1];
		if ((v.x-vert[k].x)*en[j+2].x+
			(v.y-vert[k].y)*en[j+2].y+
			(v.z-vert[k].z)*en[j+2].z>0)
			continue;
		k=trivert[f+2];
		if ((v.x-vert[k].x)*en[j+3].x+
			(v.y-vert[k].y)*en[j+3].y+
			(v.z-vert[k].z)*en[j+3].z>0)
			continue;

		ip=v;
		dist=d;
		r=i;
	}
	return r;
}

int flyFace::ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist) const 
{
	int flag=g_flyengine->shaders[sh]->flags;

	if (flag&FLY_SHADER_NOCOLLIDE)
		return -1;
	flag=!(flag&FLY_SHADER_NOCULL);

	if (facetype==FLY_FACETYPE_LARGE_POLYGON)
	{
		float x=FLY_VECDOT(normal,rd);
		if (flag && FLY_FPSIGNBIT(x)==0)
			return -1;
		dist=(d0-FLY_VECDOT(normal,ro))/x;
		if (FLY_FPSIGNBIT(dist))
			return -1;

		ip=ro+rd*dist;

		for( int i=0;i<nvert;i++ )
			if ((ip.x-vert[i].x)*en[i].x+
				(ip.y-vert[i].y)*en[i].y+
				(ip.z-vert[i].z)*en[i].z>0)
				return -1;

		return 0;
	}
	else
	{
		if (octree&&g_flyengine->octree)
			return octree->ray_intersect(ro,rd,ip,dist);
		else
		{
			int i=0,j=0,k=-1;
			flyVector v;
			float d,x;
			dist=FLY_BIG;
			for( ;i<ntrivert;i+=3,j+=4 )
			{
				x=FLY_VECDOT(en[j],rd);
				if (flag && FLY_FPSIGNBIT(x)==0)
					continue;
				d=(en[j].w - FLY_VECDOT(en[j],ro))/x;
				if (FLY_FPSIGNBIT(d) || d>=dist)
					continue;

				v=ro+rd*d;

				if ((v.x-vert[trivert[i]].x)*en[j+1].x+
					(v.y-vert[trivert[i]].y)*en[j+1].y+
					(v.z-vert[trivert[i]].z)*en[j+1].z>0)
					continue;
				if ((v.x-vert[trivert[i+1]].x)*en[j+2].x+
					(v.y-vert[trivert[i+1]].y)*en[j+2].y+
					(v.z-vert[trivert[i+1]].z)*en[j+2].z>0)
					continue;
				if ((v.x-vert[trivert[i+2]].x)*en[j+3].x+
					(v.y-vert[trivert[i+2]].y)*en[j+3].y+
					(v.z-vert[trivert[i+2]].z)*en[j+3].z>0)
					continue;

				ip=v;
				dist=d;
				k=i/3;
			}
			return k;
		}
	}
	return -1;
}

void flyFace::inverse_map(const flyVector& p, float& u, float& v) const 
{
	flyVector v1=vert[1]-vert[0],
		v2=vert[2]-vert[0],
		ip=p-vert[0];

	float det;

	det=v1.x*v2.y-v1.y*v2.x;
	if(det!=0.0f)
	{
		u=(ip.x*v2.y-ip.y*v2.x)/det;
		v=(v1.x*ip.y-v1.y*ip.x)/det;
		return;
	}

	det=v1.x*v2.z-v1.z*v2.x;
	if(det!=0.0f)
	{
		u=(ip.x*v2.z-ip.z*v2.x)/det;
		v=(v1.x*ip.z-v1.z*ip.x)/det;
		return;
	}

	det=v1.y*v2.z-v1.z*v2.y;
	u=(ip.y*v2.z-ip.z*v2.y)/det;
	v=(v1.y*ip.z-v1.z*ip.y)/det;
}

void flyFace::forward_map(float u,float v,flyVector& p) const 
{
	p.x=vert[0].x+
		(vert[1].x-vert[0].x)*u+
		(vert[2].x-vert[0].x)*v;
	p.y=vert[0].y+
		(vert[1].y-vert[0].y)*u+
		(vert[2].y-vert[0].y)*v;
	p.z=vert[0].z+
		(vert[1].z-vert[0].z)*u+
		(vert[2].z-vert[0].z)*v;
}

float flyFace::area()
{
	if (facetype==FLY_FACETYPE_LARGE_POLYGON)
	{
		int i;
		float area=0;
		flyVector v1,v2,v3;
		for( i=2;i<nvert;i++ )
		{
			v1=vert[i]-vert[0];
			v2=vert[i-1]-vert[0];
			v3.cross(v1,v2);
			area+=v3.length()*0.5f;
		}
		return area;
	}
	else
	if (facetype==FLY_FACETYPE_TRIANGLE_MESH)
	{
		int i,j=0;
		float area=0;
		flyVector v1,v2,v3;
		for( i=0;i<ntriface;i++,j+=3 )
		{
			v1=vert[trivert[j+1]]-vert[trivert[j]];
			v2=vert[trivert[j+2]]-vert[trivert[j]];
			v3.cross(v1,v2);
			area+=v3.length()*0.5f;
		}
		return area;
	}
	else
	if (facetype==FLY_FACETYPE_BEZIER_PATCH && patch)
	{
		float u,v;
		float du=0.2f/patch->nsu;
		float dv=0.2f/patch->nsv;
		flyVector p0,p1,p2;
		flyVector v1,v2,v3;
		float area=0;

		for( v=0;v<1.0f;v+=dv )
		for( u=0;u<1.0f;u+=du )
		{
			patch->evaluate(FLY_PATCHEVAL_POINT,u,v,&p0);
			patch->evaluate(FLY_PATCHEVAL_POINT,u+du,v,&p1);
			patch->evaluate(FLY_PATCHEVAL_POINT,u,v+dv,&p2);
			v1=p1-p0;
			v2=p2-p0;
			v3.cross(v1,v2);
			area+=0.5f*v3.length();
			
			patch->evaluate(FLY_PATCHEVAL_POINT,u+du,v+dv,&p0);
			v1=p1-p0;
			v2=p2-p0;
			v3.cross(v1,v2);
			area+=0.5f*v3.length();
		}
		return area;
	}

	return 0;
}