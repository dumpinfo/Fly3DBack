#include "../Fly3D.h"

// boundbox faces (6 quads) vertex indices
int flyBoundBox::facevert[6][4]=
		{	{1,7,2,4},{1,4,6,3},{1,3,5,7},
			{0,5,3,6},{0,2,7,5},{0,6,4,2}	};

// boundbox edges (12 line segments) vertex indices
int flyBoundBox::edgevert[12][2]=
		{	{0,6},{6,4},{4,2},{2,0},
			{1,3},{3,5},{5,7},{7,1},
			{0,5},{3,6},{4,1},{7,2}		};

// boundbox faces that each eage share
int flyBoundBox::edgefaces[12][2]=
		{	{0,2},{4,2},{3,2},{1,2},
			{4,5},{0,5},{1,5},{3,5},
			{0,1},{0,4},{3,4},{1,3}		};

// normalized edge vectors for each edge 
flyVector flyBoundBox::edgedir[12]= 
		{	flyVector(0,1,0),flyVector(1,0,0),flyVector(0,-1,0),flyVector(-1,0,0),
			flyVector(-1,0,0),flyVector(0,-1,0),flyVector(1,0,0),flyVector(0,1,0),
			flyVector(0,0,1),flyVector(0,0,-1),flyVector(0,0,1),flyVector(0,0,-1)	};

// normalized edge vectors for each edge divided by 100
flyVector flyBoundBox::edgedirth[12]= 
		{	flyVector(0,0.01f,0),flyVector(0.01f,0,0),flyVector(0,-0.01f,0),flyVector(-0.01f,0,0),
			flyVector(-0.01f,0,0),flyVector(0,-0.01f,0),flyVector(0.01f,0,0),flyVector(0,0.01f,0),
			flyVector(0,0,0.01f),flyVector(0,0,-0.01f),flyVector(0,0,0.01f),flyVector(0,0,-0.01f)	};

// boundbox vertex normals
flyVector flyBoundBox::vertnorm[8]=
		{	flyVector(-FLY_COS45,-FLY_COS45,-FLY_COS45),
			flyVector( FLY_COS45, FLY_COS45, FLY_COS45),
			flyVector( FLY_COS45,-FLY_COS45,-FLY_COS45),
			flyVector(-FLY_COS45, FLY_COS45, FLY_COS45),
			flyVector( FLY_COS45, FLY_COS45,-FLY_COS45),
			flyVector(-FLY_COS45,-FLY_COS45, FLY_COS45),
			flyVector(-FLY_COS45, FLY_COS45,-FLY_COS45),
			flyVector( FLY_COS45,-FLY_COS45, FLY_COS45)	};

// boundbox edge normals
flyVector flyBoundBox::edgenorm[12]=
		{	flyVector(-FLY_COS45,		 0,-FLY_COS45),
			flyVector(		  0, FLY_COS45,-FLY_COS45),
			flyVector( FLY_COS45,		 0,-FLY_COS45),
			flyVector(		  0,-FLY_COS45,-FLY_COS45),
			flyVector(		  0, FLY_COS45, FLY_COS45),
			flyVector(-FLY_COS45,		 0, FLY_COS45),
			flyVector(		  0,-FLY_COS45, FLY_COS45),
			flyVector( FLY_COS45,		 0, FLY_COS45),
			flyVector(-FLY_COS45,-FLY_COS45,		0),
			flyVector(-FLY_COS45, FLY_COS45,		0),
			flyVector( FLY_COS45, FLY_COS45,		0),
			flyVector( FLY_COS45,-FLY_COS45,		0)	};

// boundbox face normals
flyVector flyBoundBox::facenorm[6]=
		{	flyVector(1,0,0),
			flyVector(0,1,0),
			flyVector(0,0,1),
			flyVector(-1,0,0),
			flyVector(0,-1,0),
			flyVector(0,0,-1) };

// main collision method, collides the bbox in local coordinates,
// positioned at point p, moving in the normalized direction dir,
// and the leghtm of the movment is the distance len
int flyBoundBox::collide(const flyVector& p,const flyVector& dir,float& len)
{
	flyVector normal,ip;
	flyBoundBox bbobj;
	flyBspObject *obj;
	flyMesh *mesh;
	int i,j;

	g_flyengine->hitobj=0;
	g_flyengine->hitmesh=0;

	// create temporary boundbox containing all movement
	flyBoundBox bbtemp;
	bbtemp.min=min;
	bbtemp.max=max;
	bbtemp.add_point(max+dir*len);
	bbtemp.add_point(min+dir*len);
	bbtemp.min+=p;
	bbtemp.max+=p;

	// translate object boundbox into global
	min+=p;
	max+=p;

	// recurse bsp with temporay bound box
	g_flyengine->recurse_bsp_box(bbtemp.min,bbtemp.max,0);
	for( i=0;i<g_flyengine->selobjs.num;i++ )
	{
		obj=g_flyengine->selobjs[i];
		if(obj==g_flyengine->excludecollision)
			continue;

		// if dynamic object and set for collision
		if (obj->collide==0)
			continue;

		// clip object global boundbox with temporary boundbox
		bbobj.min=obj->pos+obj->bbox.min;
		bbobj.max=obj->pos+obj->bbox.max;
		if(bbtemp.clip_bbox(bbobj.min,bbobj.max)==0)
			continue;

		// get object mesh
		mesh=obj->get_mesh();
		// if dynamic object set to boundbox collision
		if (mesh==0 || obj->collide==1)
		{
			// collide with object boundbox
			if (collide(bbobj,dir,len,normal,ip))
			{
				g_flyengine->hitmesh=0;
				g_flyengine->hitobj=obj;
				g_flyengine->hitface=-1;
				g_flyengine->hitsubface=-1;
				g_flyengine->hitshader=-1;
				g_flyengine->hitnormal=normal;
				g_flyengine->hitip=ip;
			}
		}
		else 
		{
			// collide with object mesh
			if (mesh->localfaces)	// if dynamic mesh
				j=collide(mesh,obj->pos,*obj,bbtemp,dir,len,normal,ip);
			else	// else collide with static bsp mesh
				j=collide(mesh,bbtemp,dir,len,normal,ip);
			if (j!=-1)
			{
				g_flyengine->hitmesh=mesh;
				g_flyengine->hitobj=obj;
				g_flyengine->hitface=j&0xffff;
				g_flyengine->hitsubface=j>>16;
				g_flyengine->hitshader=mesh->faces[g_flyengine->hitface]->sh;
				g_flyengine->hitnormal=normal;
				g_flyengine->hitip=ip;
			}
		}
	}

	g_flyengine->hitdist=len;

	// translate boudnbox into local
	min-=p;
	max-=p;

	return (g_flyengine->hitmesh!=0 || g_flyengine->hitobj);
}

// collide ray defined by ray origin (ro) and ray direction (rd)
// with the bound box. returns -1 on no collision and the face index 
// for first intersection if a collision is found together with 
// the distances to the collision points (tnear and tfar)
int flyBoundBox::ray_intersect(const flyVector& ro,const flyVector& rd,float& tnear,float& tfar) const
{
	float t1,t2,t;
	int ret=-1;

	tnear=-FLY_BIG;
	tfar=FLY_BIG;

	int a,b;
	for( a=0;a<3;a++ )
	{
		if (rd[a]>-FLY_SMALL && rd[a]<FLY_SMALL)
			if (ro[a]<min[a] || ro[a]>max[a])
				return -1;
			else ;
		else 
		{
			t1=(min[a]-ro[a])/rd[a];
			t2=(max[a]-ro[a])/rd[a];
			if (t1>t2)
			{ 
				t=t1; t1=t2; t2=t; 
				b=3+a;
			}
			else
				b=a;
			if (t1>tnear)
			{
				tnear=t1;
				ret=b;
			}
			if (t2<tfar)
				tfar=t2;
			if (tnear>tfar || tfar<FLY_SMALL)
				return -1;
		}
	}
	
	if (tnear>tfar || tfar<FLY_SMALL)
		return -1;

	return ret;
}

// collide edge (p1,p2) moving in direction (dir) colliding
// withe edge (p3,p4). return true on a collision with 
// collision distance (dist) and intersection point (ip)
int flyBoundBox::edge_collision(const flyVector& p1,const flyVector& p2,const flyVector& dir,const flyVector& p3,const flyVector& p4,float& dist,flyVector& ip) const
{
	flyVector v1=p2-p1;
	flyVector v2=p4-p3;

	// build plane based on edge (p1,p2) and move direction (dir)
	flyVector plane;
	plane.cross(v1,dir);
	plane.normalize();
	plane.w=FLY_VECDOT(plane,p1);

	// if colliding edge (p3,p4) does not cross plane return no collision
	// same as if p3 and p4 on same side of plane return 0
	float temp=(FLY_VECDOT(plane,p3)-plane.w)*(FLY_VECDOT(plane,p4)-plane.w);
	if (FLY_FPSIGNBIT(temp)==0)
		return 0;

	// if colliding edge (p3,p4) and plane are paralell return no collision
	v2.normalize();
	temp=FLY_VECDOT(plane,v2);
	if(FLY_FPBITS(temp)==0)
		return 0;
	
	// compute intersection point of plane and colliding edge (p3,p4)
	ip=p3+v2*((plane.w-FLY_VECDOT(plane,p3))/temp);

	// find biggest 2d plane projection
	FLY_FPABS(plane.x);
	FLY_FPABS(plane.y);
	FLY_FPABS(plane.z);
	int i,j;
	if (plane.x>plane.y) i=0; else i=1;
	if (plane[i]<plane.z) i=2;
	if (i==0) { i=1; j=2; } else if (i==1) { i=0; j=2; } else { i=0; j=1; }

	// compute distance of intersection from line (ip,-dir) to line (p1,p2)
	dist=(v1[i]*(ip[j]-p1[j])-v1[j]*(ip[i]-p1[i]))/
		(v1[i]*dir[j]-v1[j]*dir[i]);
	if (FLY_FPSIGNBIT(dist)) 
		return 0;

	// compute intesection point on edge (p1,p2) line
	ip-=dist*dir;

	// check if intersection point (ip) is between egde (p1,p2) vertices
	temp=(p1.x-ip.x)*(p2.x-ip.x)+(p1.y-ip.y)*(p2.y-ip.y)+(p1.z-ip.z)*(p2.z-ip.z);
	if (FLY_FPSIGNBIT(temp))
		return 1;	// collision found!
	
	return 0; // no collision
}

// collides the boundbox moving in the direction (dir) with a mesh.
// the boundbox envolving the movement (bbtemp) and the magnitude
// of the movement (len) must be supplied.
// returns -1 if no collision is found and the collide face number 
// if a collision is found.
// on collision also returns the distance to the collision (len)
// intersection normal (normal) and intresection point (ip).
int flyBoundBox::collide(flyMesh *mesh,const flyBoundBox& bbtemp,const flyVector& dir,float& len,flyVector& normal,flyVector& ip) const
{
	int i,j,k,l,m,n,f,backfacecull,ret=-1;
	float f1,f2;
	flyVector v,p,edge1,edge2;
	flyFace *face;
	flyShader *shader;

	for(i=0;i<mesh->nf;i++)
	{
		face=mesh->faces[i];

		backfacecull=1;

		shader=g_flyengine->shaders[face->sh];
		if ((shader->npass==0 || 
			shader->flags&FLY_SHADER_NOCOLLIDE || 
			shader->flags&FLY_SHADER_SKY))
			continue;
		backfacecull=(shader->flags&FLY_SHADER_NOCULL)==0;

		if(bbtemp.clip_bbox(face->bbox.min,face->bbox.max)==0)
			continue;

		for( j=0;j<8;j++ )
			if (FLY_VECDOT(vertnorm[j],dir)>-FLY_COS45) 
			{
				f=face->ray_intersect(get_vert(j),dir,v,f1);
				if(f!=-1 && f1<len)
				{
					ret=i|(f<<16);
					len=f1;
					ip=v;
					if(face->facetype==FLY_FACETYPE_LARGE_POLYGON)
						normal=face->normal;
					else
						normal=face->en[f<<2];
				}
			}

		if(face->facetype==FLY_FACETYPE_LARGE_POLYGON)
		{
			if (backfacecull && FLY_VECDOT(face->normal,dir)>-0.01f)
				continue;

			v.vec(-dir.x,-dir.y,-dir.z);
			for( j=0;j<face->nvert;j++ )
				if (bbtemp.is_inside(face->vert[j]))
				{
					f=ray_intersect(face->vert[j],v,f1,f2);
					if (f!=-1 && f1<len && f1>=0)
						{
						ret=i;
						len=f1;
						ip=face->vert[j]+v*len;
						normal=facenorm[f];
						}
				}

			for( j=0;j<12;j++ )
			if (FLY_VECDOT(edgenorm[j],dir)>-FLY_COS45)
			{
				edge1=get_vert(edgevert[j][0]);
				edge2=get_vert(edgevert[j][1]);
				edge1+=edgedirth[j];
				edge2-=edgedirth[j];
				for( l=0;l<face->nvert;l++ )
					if (edge_collision(edge1,edge2,dir,face->vert[l],face->vert[(l+1)%face->nvert],f1,v))
						if (f1<len)
						{
							normal.cross(edgedir[j],face->vert[l]-face->vert[(l+1)%face->nvert]);
							normal.normalize();
							if (FLY_VECDOT(normal,dir)>=0)
								normal.negate();
							ret=i;
							ip=v;
							len=f1;
						}
			}
		}
		else
		{
			v.vec(-dir.x,-dir.y,-dir.z);
			if (face->octree&&g_flyengine->octree)
			{
				static flyArray<int> trifaces;
				face->octree->clip_bbox(bbtemp,trifaces);
				for( j=0;j<trifaces.num;j++ )
				{
					l=trifaces[j]*3;
					for( k=0;k<3;k++ )
					{
 					p=face->vert[face->trivert[l+k]];
					f=ray_intersect(p,v,f1,f2);
					if (f!=-1 && f1<len && f1>=0)
						{
						ret=i;
						len=f1;
						ip=p+v*len;
						normal=facenorm[f];
						}
					}
				}

				for( j=0;j<12;j++ )
				if (FLY_VECDOT(edgenorm[j],dir)>-FLY_COS45)
				{
					edge1=get_vert(edgevert[j][0]);
					edge2=get_vert(edgevert[j][1]);
					edge1+=edgedirth[j];
					edge2-=edgedirth[j];
					for( n=0;n<trifaces.num;n++ )
						if (FLY_VECDOT(face->en[trifaces[n]<<2],dir)<0.01f)
						{
							l=trifaces[n]*3;
							for( m=0;m<3;m++ )
								if (edge_collision(edge1,edge2,dir,face->vert[face->trivert[l+m]],face->vert[face->trivert[l+((m+1)%3)]],f1,v))
									if (f1<len)
									{
										normal.cross(edgedir[j],face->vert[face->trivert[l+m]]-face->vert[face->trivert[l+((m+1)%3)]]);
										normal.normalize();
										if (FLY_VECDOT(normal,dir)>=0)
											normal.negate();
										ret=i;
										ip=v;
										len=f1;
									}
						}
				}
			}
			else
			{
				for( j=0;j<face->nvert;j++ )
				if (bbtemp.is_inside(face->vert[j+face->vertindx]))
				{
					f=ray_intersect(face->vert[j+face->vertindx],v,f1,f2);
					if (f!=-1 && f1<len && f1>=0)
					{
						ret=i;
						len=f1;
						ip=face->vert[j+face->vertindx]+v*len;
						normal=facenorm[f];
					}
				}

				for( j=0;j<12;j++ )
				if (FLY_VECDOT(edgenorm[j],dir)>-FLY_COS45)
				{
					edge1=get_vert(edgevert[j][0]);
					edge2=get_vert(edgevert[j][1]);
					edge1+=edgedirth[j];
					edge2-=edgedirth[j];
					for( l=n=0;l<face->ntrivert;l+=3,n+=4 )
						if (FLY_VECDOT(face->en[n],dir)<0.01f)
							for( m=0;m<3;m++ )
								if (edge_collision(edge1,edge2,dir,face->vert[face->trivert[l+m]],face->vert[face->trivert[l+((m+1)%3)]],f1,v))
									if (f1<len)
									{
										normal.cross(edgedir[j],face->vert[face->trivert[l+m]]-face->vert[face->trivert[l+((m+1)%3)]]);
										normal.normalize();
										if (FLY_VECDOT(normal,dir)>=0)
											normal.negate();
										ret=i;
										ip=v;
										len=f1;
									}
				}
			}
		}
	}
	return ret;
}

// collides the boundbox moving in the direction (dir) with a 
// mesh at position pos and with rotation defined by ls.
// the boundbox envolving the movement (bbtemp) and the magnitude
// of the movement (len) must be supplied.
// returns -1 if no collision is found and the collide face number 
// if a collision is found.
// on collision also returns the distance to the collision (len)
// intersection normal (normal) and intresection point (ip).
int flyBoundBox::collide(flyMesh *mesh,const flyVector& pos,const flyLocalSystem& ls,const flyBoundBox& bbtemp,const flyVector& dir,float& len,flyVector& normal,flyVector& ip) const
{
	int i,j,k,l,m,n,f,backfacecull,ret=-1;
	float f1,f2;
	flyVector v,edge1,edge2;
	flyVector vv1,vv2;
	flyFace *face;
	flyShader *shader;

	flyBoundBox bbtemp2;
	bbtemp2.reset();
	for( i=0;i<8;i++ )
		bbtemp2.add_point((bbtemp.get_vert(i)-pos)*ls.mat_t);

	for(i=0;i<mesh->nf;i++)
	{
		face=mesh->faces[i];

		backfacecull=1;

		shader=g_flyengine->shaders[face->sh];
		if ((shader->npass==0 || 
			shader->flags&FLY_SHADER_NOCOLLIDE || 
			shader->flags&FLY_SHADER_SKY))
			continue;
		backfacecull=(shader->flags&FLY_SHADER_NOCULL)==0;

		if(bbtemp2.clip_bbox(face->bbox.min,face->bbox.max)==0)
			continue;

		vv2=dir*ls.mat_t;
		for( j=0;j<8;j++ )
			if (FLY_VECDOT(vertnorm[j],dir)>-FLY_COS45) 
			{
				vv1=(get_vert(j)-pos)*ls.mat_t;
				f=face->ray_intersect(vv1,vv2,v,f1);
				if(f!=-1 && f1<len)
				{
					ret=i;
					len=f1;
					ip=v*ls.mat+pos;
					if(face->facetype==FLY_FACETYPE_LARGE_POLYGON)
						normal=face->normal*ls.mat;
					else
						normal=face->en[f<<2]*ls.mat;
				}
			}

		v.x=-dir.x; v.y=-dir.y; v.z=-dir.z;
		if (face->octree&&g_flyengine->octree)
		{
			static flyArray<int> trifaces;
			face->octree->clip_bbox(bbtemp2,trifaces);

			for( j=0;j<trifaces.num;j++ )
			{
				l=trifaces[j]*3;
				for( k=0;k<3;k++ )
				{
 				vv1=face->vert[face->trivert[l+k]]*ls.mat+pos;
				f=ray_intersect(vv1,v,f1,f2);
				if (f!=-1 && f1<len && f1>=0)
					{
					ret=i;
					len=f1;
					ip=vv1+v*len;
					normal=facenorm[f];
					}
				}
			}

			for( j=0;j<12;j++ )
			if (FLY_VECDOT(edgenorm[j],dir)>-FLY_COS45)
			{
				edge1=get_vert(edgevert[j][0]);
				edge2=get_vert(edgevert[j][1]);
				edge1+=edgedirth[j];
				edge2-=edgedirth[j];
				for( n=0;n<trifaces.num;n++ )
				{
					vv1=face->en[trifaces[n]<<2]*ls.mat;
					if (FLY_VECDOT(vv1,dir)<0.01f)
					{
						l=trifaces[n]*3;
						for( m=0;m<3;m++ )
						{
							vv1=face->vert[face->trivert[l+m]]*ls.mat+pos;
							vv2=face->vert[face->trivert[l+((m+1)%3)]]*ls.mat+pos;
							if (edge_collision(edge1,edge2,dir,vv1,vv2,f1,v))
								if (f1<len)
								{
									normal.cross(edgedir[j],vv1-vv2);
									normal.normalize();
									if (FLY_VECDOT(normal,dir)>=0)
										normal.negate();
									ret=i;
									ip=v;
									len=f1;
								}
						}
					}
				}
			}
		}
		else
		{
			for( j=0;j<face->nvert;j++ )
			{
				vv1=face->vert[j+face->vertindx]*ls.mat+pos;
				if (bbtemp.is_inside(edge1))
				{
					f=ray_intersect(vv1,v,f1,f2);
					if (f!=-1 && f1<len && f1>=0)
					{
						ret=i;
						len=f1;
						ip=vv1+v*len;
						normal=facenorm[f];
					}
				}
			}

			for( j=0;j<12;j++ )
			if (FLY_VECDOT(edgenorm[j],dir)>-FLY_COS45)
			{
				edge1=get_vert(edgevert[j][0]);
				edge2=get_vert(edgevert[j][1]);
				edge1+=edgedirth[j];
				edge2-=edgedirth[j];
				for( l=n=0;l<face->ntrivert;l+=3,n+=4 )
				{
					vv1=face->en[n]*ls.mat;
					if (FLY_VECDOT(vv1,dir)<0.01f)
						for( m=0;m<3;m++ )
						{
							vv1=face->vert[face->trivert[l+m]]*ls.mat+pos;
							vv2=face->vert[face->trivert[l+((m+1)%3)]]*ls.mat+pos;
							if (edge_collision(edge1,edge2,dir,vv1,vv2,f1,v))
								if (f1<len)
								{
									normal.cross(edgedir[j],vv1-vv2);
									normal.normalize();
									normal=normal;
									if (FLY_VECDOT(normal,dir)>=0)
										normal.negate();
									ret=i;
									ip=v;
									len=f1;
								}
						}
				}
			}
		}
	}

	return ret;
}

// collides the boundbox moving in the direction (dir) and 
// with movement magnitude (len) with another boundbox (bbox).
// returns true on collision together with the intersection
// distance (len), intersection normal (normal) and 
// intersection point (ip).
int flyBoundBox::collide(const flyBoundBox& bbox,const flyVector& dir,float& len,flyVector& normal,flyVector& ip) const
{
	static flyVector v,edge1,edge2,edge3,edge4;
	int i,j,f,ret=0;
	float f1,f2,dot;

	// for all bbox vertices
	for( i=0;i<8;i++ )
	{
		dot=FLY_VECDOT(vertnorm[i],dir);

		// test vertices from source bbox to destination bbox
		if(dot>-FLY_COS45)
		{
			v=get_vert(i);
			f=bbox.ray_intersect(v,dir,f1,f2);
			if (f!=-1 && f1<len && f1>=0)
			{
				ret=1;
				len=f1;
				ip=v+dir*len;
				f=f>2?f-3:f+3;
				normal=facenorm[f];
			}
		}

		// test vertices from dest bbox to source bbox 
		if(-dot>-FLY_COS45)
		{
			v=bbox.get_vert(i);
			f=ray_intersect(v,flyVector(-dir.x,-dir.y,-dir.z),f1,f2);
			if (f!=-1 && f1<len && f1>=0)
			{
				ret=1;
				len=f1;
				ip=v-dir*len;
				normal=facenorm[f];
			}
		}
	}

	// test edges between the source bbox and destination bbox
	for( i=0;i<12;i++ )
	{
		if (FLY_VECDOT(edgenorm[i],dir)>-FLY_COS45)
		{
			edge1=get_vert(edgevert[i][0]);
			edge2=get_vert(edgevert[i][1]);
			edge1+=edgedirth[i];
			edge2-=edgedirth[i];
			for( j=0;j<12;j++ )
			{
				if (-(FLY_VECDOT(edgenorm[j],dir))>-FLY_COS45)
				{
					edge3=bbox.get_vert(edgevert[j][0]);
					edge4=bbox.get_vert(edgevert[j][1]);
					edge3+=edgedirth[j];
					edge4-=edgedirth[j];
					if (edge_collision(edge1,edge2,dir,edge3,edge4,f1,v))
					if (f1<len)
					{
						ret=1;
						ip=v;
						len=f1;
						normal.cross(edgedir[i],edgedir[j]);
						if (FLY_VECDOT(normal,dir)>=0)
							normal.negate();
					}
				}
			}
		}
	}

	return ret;
}

void flyBoundBox::add_point(const flyVector& p)
{
	if (p.x>max.x)
		max.x=p.x;
	if (p.y>max.y)
		max.y=p.y;
	if (p.z>max.z)
		max.z=p.z;

	if (p.x<min.x)
		min.x=p.x;
	if (p.y<min.y)
		min.y=p.y;
	if (p.z<min.z)
		min.z=p.z;
}

flyMesh *flyBoundBox::get_mesh() const 
{
	static int vertarray[36]=
	{ 0,1,2, 0,2,3, 4,6,5, 4,7,6,
	0,4,5, 0,5,1, 1,5,6, 1,6,2, 2,6,7, 2,7,3, 3,7,4, 3,4,0 };
	float f=(float)(1.0/sqrt(3.0));
	flyMesh *m=new flyMesh;

	m->set_numverts(8);
	m->set_numfaces(1,1);
	m->set_numen(48);

	m->localvert[0].vec(max.x,max.y,min.z);
	m->localvert[1].vec(max.x,min.y,min.z);
	m->localvert[2].vec(min.x,min.y,min.z);
	m->localvert[3].vec(min.x,max.y,min.z);

	m->localvert[4].vec(max.x,max.y,max.z);
	m->localvert[5].vec(max.x,min.y,max.z);
	m->localvert[6].vec(min.x,min.y,max.z);
	m->localvert[7].vec(min.x,max.y,max.z);

	m->localvert[0].normal.vec(1,1,-1);
	m->localvert[1].normal.vec(1,-1,-1);
	m->localvert[2].normal.vec(-1,-1,-1);
	m->localvert[3].normal.vec(-1,1,-1);

	m->localvert[4].normal.vec(1,1,1);
	m->localvert[5].normal.vec(1,-1,1);
	m->localvert[6].normal.vec(-1,-1,1);
	m->localvert[7].normal.vec(-1,1,1);

	m->localfaces[0].facetype=FLY_FACETYPE_TRIANGLE_MESH;
	m->localfaces[0].patch=0;
	m->localfaces[0].lm=-1;
	m->localfaces[0].sh=-1;
	m->localfaces[0].indx=0;
	m->localfaces[0].nvert=8;
	m->localfaces[0].vertindx=0;
	m->localfaces[0].ntriface=12;
	m->localfaces[0].ntrivert=36;
	m->localfaces[0].trivert=new int[36];
	m->localfaces[0].vert=m->localvert;
	m->localfaces[0].en=m->localen;
	memcpy(m->localfaces[0].trivert,vertarray,sizeof(int)*36);

	m->compute_normals();

	return m;
}

void flyBoundBox::draw() const 
{
	flyVector v;
	g_flytexcache->sel_tex(-1);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_LINES);
	for( int i=0;i<12;i++ )
	{
		v=get_vert(edgevert[i][0]);
		glVertex3fv(&v.x);
		v=get_vert(edgevert[i][1]);
		glVertex3fv(&v.x);
	}
	glEnd();
}
