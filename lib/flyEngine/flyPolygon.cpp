#include "../Fly3D.h"

flyPolygon::flyPolygon()
{
}

flyPolygon::flyPolygon(const flyPolygon& in) :
	flyPlane(in),
	verts(in.verts),
	edgeplanes(in.edgeplanes)
{ 
}

void flyPolygon::operator=(const flyPolygon& in)
{ 
	flyPlane::operator=(in);
	verts=in.verts;
	edgeplanes=in.edgeplanes;
}

flyPolygon::~flyPolygon()
{
}

void flyPolygon::clip_edge(int i,const flyPlane& plane,flyVertex& v)
{
	flyVector ro=verts[i];
	flyVector rd=verts[(i+1)%verts.num]-ro;
	float x=FLY_VECDOT(plane.normal,rd);
	float dist=(plane.d0-FLY_VECDOT(plane.normal,ro))/x;
	v.x=ro.x+dist*rd.x;
	v.y=ro.y+dist*rd.y;
	v.z=ro.z+dist*rd.z;
	v.texcoord.x=verts[i].texcoord.x+dist*(verts[(i+1)%verts.num].texcoord.x-verts[i].texcoord.x);
	v.texcoord.y=verts[i].texcoord.y+dist*(verts[(i+1)%verts.num].texcoord.y-verts[i].texcoord.y);
}

void flyPolygon::clip_plane(const flyPlane& plane)
{
	flyArray<flyVertex> tmpverts;
	flyArray<float> tmpdists;
	flyVertex v;

	int i,n=verts.num;
	
	for( i=0;i<n;i++ )
		tmpdists.add(plane.distance(verts[i]));

	for( i=0;i<n;i++ )
	if (tmpdists[i]<0 || tmpdists[(i+1)%n]<0)
	{
		if (tmpdists[i]<0 && tmpdists[(i+1)%n]<0)
			tmpverts.add(verts[i]);
		else
		if (tmpdists[i]<0)
		{
			tmpverts.add(verts[i]);
			clip_edge(i,plane,v);
			tmpverts.add(v);
		}
		else
		{
			clip_edge(i,plane,v);
			tmpverts.add(v);
		}
	}

	verts=tmpverts;
}

void flyPolygon::draw()
{
	glBegin(GL_POLYGON);
	for( int i=0;i<verts.num;i++ )
	{
		glTexCoord2fv(&verts[i].texcoord.x);
		glVertex3fv(&verts[i].x);
	}
	glEnd();
}

void flyPolygon::build_quad(const flyVector& pos,const flyVector& x,const flyVector& y,const flyVector& z)
{
	flyVertex v;
	
	verts.clear();
	v.normal=z;
	v.color=0xffffffff;

	v.vec(pos.x+x.x-y.x-z.x, pos.y+x.y-y.y-z.y, pos.z+x.z-y.z-z.z);
	v.texcoord.vec(1,0,0,0);
	verts.add(v);
	v.vec(pos.x+x.x+y.x-z.x, pos.y+x.y+y.y-z.y, pos.z+x.z+y.z-z.z);
	v.texcoord.vec(1,1,0,0);
	verts.add(v);
	v.vec(pos.x+y.x-x.x-z.x, pos.y+y.y-x.y-z.y, pos.z+y.z-x.z-z.z);
	v.texcoord.vec(0,1,0,0);
	verts.add(v);
	v.vec(pos.x-x.x-y.x-z.x, pos.y+-x.y-y.y-z.y, pos.z-x.z-y.z-z.z);
	v.texcoord.vec(0,0,0,0);
	verts.add(v);
}

float clockwise_dot(const flyVector &v1,const flyVector &v2,const flyVector &normal)
{
	float dot=FLY_VECDOT(v1,v2),f;
	flyVector v;
	v.cross(v1,v2);
	f=FLY_VECDOT(v,normal);

	if(FLY_FPSIGNBIT(f))
		dot=-dot-2;

	return dot;
}

void flyPolygon::order_verts()
{
	int i,j,next;
	float maxdot,f;
	flyVector centre,v0,v1;

	// find polygon center
	centre.null();
	for(i=0;i<verts.num;i++)
		centre+=verts[i];
	centre*=1.0f/verts.num;

	// find vector for first vertex
	v0=verts[0]-centre;
	v0.normalize();
	
	// for every other vertex
	for(i=0;i<verts.num-2;i++)
	{
		// find vertex with biggest clockwise dotproduct
		maxdot=-4.0f;
		next=-1;
		for(j=i+1;j<verts.num;j++)
		{
			v1=verts[j]-centre;
			v1.normalize();
			if((f=clockwise_dot(v0,v1,normal))>maxdot)
			{
				maxdot=f;
				next=j;
			}
		}

		// swap vertex to correct order
		flyVertex aux=verts[i+1];
		verts[i+1]=verts[next];
		verts[next]=aux;
	}
}

void flyPolygon::build_edgeplanes()
{
	// build a plane for each polygon edge parallel to polygon normal
	int i;
	flyVector edge;
	flyPlane plane;

	edgeplanes.clear();
	for(i=0;i<verts.num;i++)
	{
		edge=(flyVector)(verts[(i+1)%verts.num]-verts[i]);
		plane.normal.cross(normal,edge);
		plane.normal.normalize();
		plane.d0=FLY_VECDOT(plane.normal,verts[i]);
		edgeplanes.add(plane);
	}
}

int flyPolygon::intersect(flyPolygon &in, flyPolygon &out)
{
	// clear output polygon
	out.verts.clear();
	
	// find vertices from polygon in that are inside current polygon
	build_edgeplanes();
	intersect_verts(in,out);

	// find vertices current polygon that are inside polygon in
	in.build_edgeplanes();
	in.intersect_verts(*this,out);

	// find all intersections from edges of the current polygon
	// with edges from the in polygon
	intersect_edges(in,out);

	// return number of vertices in output polygon 
	// >2 means a valid polygon
	return out.verts.num;
}

int flyPolygon::intersect_verts(const flyPolygon &in, flyPolygon &out)
{
	int i,j,k;

	// for all vertices from in polygon
	for(i=0;i<in.verts.num;i++)
	{
		// test if distance to current polygon edges is smaller then threshold 
		// (edge plane normal points inside polygon)
		for(j=0;j<edgeplanes.num;j++)
			if(edgeplanes[j].distance(in.verts[i])<-0.1f)
				break;
		// if passed all above tests, vertex is inside current polygon
		if(j==edgeplanes.num)
		{
			// test if vertex is not already in output list
			for(k=0;k<out.verts.num;k++)
				if((out.verts[k]-in.verts[i]).length2()<0.1f)
					break;
			// if not in output list, add it
			if(k==out.verts.num)
				out.verts.add(in.verts[i]);
		}
	}
	
	return out.verts.num;
}

int flyPolygon::intersect_edges(const flyPolygon &in, flyPolygon &out)
{
	int i,j,k,a,b;
	float f1,f2;
	flyPlane plane;
	flyVector p1,p2,d1,d2;

	// for every edge from current polygon
	for(i=0;i<verts.num;i++)
		// for every edge from in polygon
		for(j=0;j<in.verts.num;j++)
		{
			// test1: check if in edge vertices are on same plane of current polygon edge
			f1=edgeplanes[i].distance(in.verts[j]);
			f2=edgeplanes[i].distance(in.verts[(j+1)%in.verts.num]);
			if((f1*f2)>-0.1f)
				continue;

			// test2: check if current edge vertices are on same plane of in polygon edge
			f1=in.edgeplanes[j].distance(verts[i]);
			f2=in.edgeplanes[j].distance(verts[(i+1)%verts.num]);
			if((f1*f2)>-0.1f)
				continue;
			
			// pased test 1 and test2: intersection exists
			
			// compute edge vector d1
			p1=verts[i];
			d1=verts[(i+1)%verts.num]-p1;
			d1.normalize();

			// compute edge vector d2
			p2=in.verts[j];
			d2=in.verts[(j+1)%in.verts.num]-p2;
			d2.normalize();

			// project onto maximum area plane
			plane.normal.cross(d1,d2);
			plane.normal.normalize();
			plane.d0=FLY_VECDOT(plane.normal,verts[i]);
			FLY_FPABS(plane.normal.x);
			FLY_FPABS(plane.normal.y);
			FLY_FPABS(plane.normal.z);
			if (plane.normal.x>plane.normal.y) a=0; else a=1;
			if (plane.normal[a]<plane.normal.z) a=2;
			if (a==0) { a=1; b=2; } else if (a==1) { a=0; b=2; } else { a=0; b=1; }

			// compute distance of intersection 
			float dist=	(d2[a]*(p1[b]-p2[b])-d2[b]*(p1[a]-p2[a]))/
						(d2[b]*d1[a]-d2[a]*d1[b]);

			// compute intersection point
			p1+=dist*d1;

			flyVertex v;
			v.x=p1.x;
			v.y=p1.y;
			v.z=p1.z;

			// test if vertex in not already in output list
			for(k=0;k<out.verts.num;k++)
				if((out.verts[k]-v).length2()<0.1f)
					break;
			// if not in output list, add it
			if(k==out.verts.num)
				out.verts.add(v);
		}
	
	return out.verts.num;
}

float flyPolygon::area()
{
	float area=0.0f;
	flyVector v1,v2,v3;
	int i;

	for( i=0;i<verts.num-2;i++ )
	{
		v1=verts[i+1]-verts[0];
		v2=verts[i+2]-verts[0];
		v3.cross(v1,v2);
		area+=v3.length()*0.5f;
	}
	
	return area;
}

int flyPolygon::random_point(flyVector& point,float threshold)
{
	flyBoundBox bb;
	flyVector v;
	int i,j,k;

	bb.reset();
	for( i=0;i<verts.num;i++ )
		bb.add_point(verts[i]);
	flyVector bbdiag=bb.max-bb.min;
	flyVector randvec;

	for( k=0;k<64;k++ )
	{
		randvec.vec((rand()%10000)*0.0001f,(rand()%10000)*0.0001f,(rand()%10000)*0.0001f);
		v=bb.min+bbdiag*randvec;
		v-=distance(v)*normal;
		for(j=0;j<edgeplanes.num;j++)
			if(edgeplanes[j].distance(v)<threshold)
				break;
		if (j==edgeplanes.num)
			break;
	}
	if (k==64)
		return 0;

	point=v;
	return 1;
}

void flyPolygon::build_normal()
{
	if (verts.num<3)
	{
		normal.vec(0,0,0,0);
		d0=0;
	}
	else
	{
		flyVector v1=verts[1]-verts[0];
		flyVector v2=verts[2]-verts[0];
		normal.cross(v1,v2);
		normal.normalize();
		d0=FLY_VECDOT(normal,verts[0]);
	}
}

flyVector flyPolygon::get_center() const
{
	flyVector v(0);

	for(int i=0;i<verts.num;i++)
		v+=verts[i];
	v*=1.0f/i;

	return v;
}

void flyPolygon::add(const flyVector& v)
{
	flyVertex vert;
	vert.vec(v.x,v.y,v.z);
	verts.add(vert);
}

void flyPolygon::add(const flyVertex& v)
{
	verts.add(v);
}

void flyPolygon::clear()
{
	verts.clear();
	edgeplanes.clear();
}
