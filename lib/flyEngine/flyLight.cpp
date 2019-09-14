#include "..\Fly3D.h"

flyLightVertex::flyLightVertex() : 
	nlights(0),frame(0)
{ 
}

flyLightVertex::flyLightVertex(const flyLightVertex& in) :
	nlights(in.nlights),frame(in.frame)
{
	for( int i=0;i<nlights;i++ )
	{
		pos[i]=in.pos[i];
		color[i]=in.color[i];
		radius[i]=in.radius[i];
	}
}

flyLightVertex::~flyLightVertex() 
{
}

void flyLightVertex::operator=(const flyLightVertex& in) 
{
	nlights=in.nlights;
	frame=in.frame;
	for( int i=0;i<nlights;i++ )
	{
		pos[i]=in.pos[i];
		color[i]=in.color[i];
		radius[i]=in.radius[i];
	}
}

void flyLightVertex::add_light(const flyVector& p,const flyVector& c,float r)
{
	if (frame<g_flyengine->cur_frame_base)
	{
		nlights=0;
		frame=g_flyengine->cur_frame_base;
	}
	if (nlights<FLY_MAX_HWLIGHTS)
	{
		pos[nlights]=p;
		pos[nlights].w=1.0f;
		color[nlights]=c;
		color[nlights].w=1.0f;
		radius[nlights]=r;
		nlights++;
	}
}

void flyLightVertex::add_light(const flyVector& p,const flyVector& c,float r,const flyVector& objpos)
{
	if (frame<g_flyengine->cur_frame_base)
	{
		nlights=0;
		frame=g_flyengine->cur_frame_base;
	}
	float dist=(p-objpos).length2(),d;
	int i;
	for( i=0;i<nlights;i++ )
	{
		d=(pos[i]-objpos).length2();
		if (dist<d)
		{
			int j;
			if (nlights==FLY_MAX_HWLIGHTS)
				j=nlights-2;
			else
				j=nlights-1;
			for( ;j>=i;j-- )
			{
				pos[j+1]=pos[j];
				color[j+1]=color[j];
				radius[j+1]=radius[j];
			}
			break;
		}

	}
	if (i<FLY_MAX_HWLIGHTS)
	{
		pos[i]=p;
		pos[i].w=1.0f;
		color[i]=c;
		color[i].w=1.0f;
		radius[i]=r;
		if (nlights<FLY_MAX_HWLIGHTS)
			nlights++;
	}
}

void flyLightVertex::init_draw(const flyBspObject *obj) const
{
	glEnable(GL_LIGHTING);
	int i;
	for( i=0;i<g_flyrender->m_numhwlights;i++ )
	{
		if (i>=nlights)
			glDisable(GL_LIGHT0+i);
		else 
		{
			glEnable(GL_LIGHT0+i);
			glLightfv(GL_LIGHT0+i,GL_POSITION,&pos[i].x);
			glLightfv(GL_LIGHT0+i,GL_DIFFUSE,&color[i].x);
			glLightfv(GL_LIGHT0+i,GL_SPECULAR,&color[i].x);
			glLightf(GL_LIGHT0+i,GL_LINEAR_ATTENUATION,1.0f/radius[i]);
		}
	}	
	flyVector ambient;
	ambient.null();
	if (obj)
	{
		if (obj->clipnodes.num)
		{
			for( i=0;i<obj->clipnodes.num;i++ )
				ambient+=obj->clipnodes[i]->color;
			ambient*=1.0f/obj->clipnodes.num;
			ambient.w=1;
			glLightModelfv(GL_LIGHT_MODEL_AMBIENT,&ambient.x);
		}
		if (nlights && NV_vertex_program)
		{
			i=get_closest(obj->pos);
			flyVector p(pos[i]-obj->pos);
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 30,p.x,p.y,p.z,0);
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 31,color[i].x,color[i].y,color[i].z,0);
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 32,radius[i],radius[i],radius[i],0);
			
			p.normalize();
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 33,p.x,p.y,p.z,0 );
			p=(p+obj->Z)*0.5f;
			p.normalize();
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 34,p.x,p.y,p.z,32 );
			glProgramParameter4fNV(GL_VERTEX_PROGRAM_NV, 35,ambient.x,ambient.y,ambient.z,0 );
		}
	}
}

int flyLightVertex::get_closest(const flyVector& p) const
{
	int i,min_i=-1;
	float f,min_f=1e10;
	for( i=0;i<nlights;i++ )
	{
		f=p.distto2(pos[i]);
		if (f<min_f)
		{
			min_f=f;
			min_i=i;
		}
	}
	return min_i;
}

flyLightMap::~flyLightMap() 
{ 
	delete[] bmp; 
};

flyLightMap::flyLightMap(int f,int p,int x,int y,int sx,int sy,int bp)
{
	facenum=f;
	pic=p;
	offsetx=x;
	offsety=y;
	sizex=sx;
	sizey=sy;
	bytespixel=bp;
	lastupdate=0;
	bytesx=sx*bp;
	bytesxy=bytesx*sy;
	bmp=new unsigned char[bytesxy];
};

void flyLightMap::set_base(const flyFace *f,const flyLightMapPic *lmp,const flyVector& pos)
{
	v0 = f->vert[0] + pos;
	v1 = f->vert[1] - f->vert[0];
	v2 = f->vert[2] - f->vert[0];
	uv[0][0]=f->vert[0].texcoord.z;
	uv[0][1]=f->vert[0].texcoord.w;
	uv[1][0]=f->vert[1].texcoord.z-f->vert[0].texcoord.z;
	uv[1][1]=f->vert[1].texcoord.w-f->vert[0].texcoord.w;
	uv[2][0]=f->vert[2].texcoord.z-f->vert[0].texcoord.z;
	uv[2][1]=f->vert[2].texcoord.w-f->vert[0].texcoord.w;
	det=uv[1][0]*uv[2][1]-uv[2][0]*uv[1][1];
	normal=f->normal;
	flyVector p1,p2;
	map_point((float)(offsetx)/lmp->sizex,(float)(offsety)/lmp->sizey,d0);
	map_point((float)(offsetx+sizex)/lmp->sizex,(float)(offsety)/lmp->sizey,p1);
	map_point((float)(offsetx)/lmp->sizex,(float)(offsety+sizey)/lmp->sizey,p2);
	d1=p1-d0;
	d2=p2-d0;
}

void flyLightMap::illum(const flyVector& pos,const flyVector& color,float radius,int shadows)
{
	int i,j,k;
	unsigned char *uc;
	
	float fi=1.0f/sizex,fj=1.0f/sizey,fu,fv;
	float dist,dot,r=radius*radius;
	flyVector p,n,dir;

	fv=fj*0.5f;
	for( j=0;j<sizey;j++,fv+=fj )
	{
	fu=fi*0.5f;
	uc=&bmp[j*bytesx];
	for( i=0;i<sizex;i++,fu+=fi )
		{
		map_point_local(fu,fv,p,n);
		dir=p-pos;
		dist=dir.length2();
		if (dist>r)
			uc+=3;
		else 
			{
			dist=(float)sqrt(dist);
			dir*=1.0f/dist;
			dot=FLY_VECDOT(dir,normal);
			if ((shadows&1) && dot>0)
				{
				uc+=3;
				continue;
				}
			if (shadows&8 &&
				g_flyengine->collision_test(pos,p-dir,FLY_TYPE_STATICMESH))
				{
				uc+=3;
				continue;
				}
			dist=(1.0f-dist/radius);
			if (shadows&2)
				dist*=dist;
			if (shadows&4)
				dist*=-dot;
			dist*=255.0;

			k=(int)(color.x*dist)+(int)(*uc);
			*(uc++)=k>255?255:k;

			k=(int)(color.y*dist)+(int)(*uc);
			*(uc++)=k>255?255:k;

			k=(int)(color.z*dist)+(int)(*uc);
			*(uc++)=k>255?255:k;
			}
		}
	}
}

void flyLightMap::map_point_local(float u, float v, flyVector &p,flyVector& n) const
{ 
	static flyVector pn[2];
	if (facenum!=-1)
	switch(g_flyengine->faces[facenum].facetype)
	{
		case FLY_FACETYPE_LARGE_POLYGON:
			p=d0+d1*u+d2*v; 
			n=normal;
			break;
		case FLY_FACETYPE_BEZIER_PATCH:
			{
			if (sizex>1)
				u=(u-0.5f/sizex)*sizex/(sizex-1);
			if (sizey>1)
				v=(v-0.5f/sizey)*sizey/(sizey-1);
			if (g_flyengine->faces[facenum].patch)
				{
				g_flyengine->faces[facenum].patch->evaluate(FLY_PATCHEVAL_POINT|FLY_PATCHEVAL_NORMAL,u,v,pn);
				p=pn[0];
				n=pn[1];
				}
			}
			break;
		case FLY_FACETYPE_TRIANGLE_MESH:
			p.null();
			n.null();
			break;
	}
}

void flyLightMap::map_point(float u, float v, flyVector &point) const 
{
	u-=uv[0][0];
	v-=uv[0][1];
	point=
		v0+
		v1*((u*uv[2][1]-uv[2][0]*v)/det)+
		v2*((uv[1][0]*v-u*uv[1][1])/det);
}

void flyLightMap::load(const flyLightMapPic *lmp)
{
	int i,j=sizex*bytespixel;
	for( i=0;i<sizey;i++ )
		memcpy(
			&bmp[i*bytesx],
			&lmp->bmp[(i+offsety)*lmp->bytesx+offsetx*lmp->bytespixel],
			j);
}

void flyLightMap::save(flyLightMapPic *lmp) const 
{
	int i,j=sizex*bytespixel;
	for( i=0;i<sizey;i++ )
		memcpy(
			&lmp->bmp[(i+offsety)*lmp->bytesx+offsetx*lmp->bytespixel],
			&bmp[i*bytesx],
			j);
}

flyLightMapPic::~flyLightMapPic() 
{ 
	delete[] bmp; 
};

flyLightMapPic::flyLightMapPic(int sx,int sy,int bp)
{
	bytespixel=bp;
	sizex=sx; 
	sizey=sy; 
	bytesx=sx*bytespixel;
	bytesxy=bytesx*sizey;
	bmp=new unsigned char [bytesxy];
};

