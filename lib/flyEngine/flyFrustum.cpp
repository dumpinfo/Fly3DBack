#include "../Fly3D.h"

void flyFrustum::build(const flyVector& pos,const flyVector& X,const flyVector& Y,const flyVector& Z,float camangle,float aspect,float farplane)
{
	float disty=farplane*(float)tan(camangle*0.5f*FLY_PIOVER180);
	float distx=disty*aspect;

	verts[0]=pos;
	
	verts[1].x = pos.x - farplane*Z.x + distx*X.x + disty*Y.x;
	verts[1].y = pos.y - farplane*Z.y + distx*X.y + disty*Y.y;
	verts[1].z = pos.z - farplane*Z.z + distx*X.z + disty*Y.z;

	verts[2].x = pos.x - farplane*Z.x + distx*X.x - disty*Y.x;
	verts[2].y = pos.y - farplane*Z.y + distx*X.y - disty*Y.y;
	verts[2].z = pos.z - farplane*Z.z + distx*X.z - disty*Y.z;

	verts[3].x = pos.x - farplane*Z.x - distx*X.x - disty*Y.x;
	verts[3].y = pos.y - farplane*Z.y - distx*X.y - disty*Y.y;
	verts[3].z = pos.z - farplane*Z.z - distx*X.z - disty*Y.z;

	verts[4].x = pos.x - farplane*Z.x - distx*X.x + disty*Y.x;
	verts[4].y = pos.y - farplane*Z.y - distx*X.y + disty*Y.y;
	verts[4].z = pos.z - farplane*Z.z - distx*X.z + disty*Y.z;

	planes[0].cross(verts[2]-verts[1],verts[0]-verts[1]);
	planes[0].normalize();
	planes[0].w=FLY_VECDOT(verts[0],planes[0]);

	planes[1].cross(verts[3]-verts[2],verts[0]-verts[2]);
	planes[1].normalize();
	planes[1].w=FLY_VECDOT(verts[0],planes[1]);

	planes[2].cross(verts[4]-verts[3],verts[0]-verts[3]);
	planes[2].normalize();
	planes[2].w=FLY_VECDOT(verts[0],planes[2]);

	planes[3].cross(verts[1]-verts[4],verts[0]-verts[4]);
	planes[3].normalize();
	planes[3].w=FLY_VECDOT(verts[0],planes[3]);

	planes[4].cross(verts[3]-verts[1],verts[2]-verts[1]);
	planes[4].normalize();
	planes[4].w=FLY_VECDOT(verts[1],planes[4]);

	static int table[8][3]=
		{ {4,5,6},{4,5,2},{4,1,6},{4,1,2},
		{0,5,6},{0,5,2},{0,1,6},{0,1,2} };
	int i,j;
	for( i=0;i<5;i++ )
	{
		j =	(FLY_FPSIGNBIT(planes[i].x)>>29)|
			(FLY_FPSIGNBIT(planes[i].y)>>30)|
			(FLY_FPSIGNBIT(planes[i].z)>>31);
		bboxindx[i][0]=table[j][0];
		bboxindx[i][1]=table[j][1];
		bboxindx[i][2]=table[j][2];
	}
}

int flyFrustum::clip_bbox(const flyBoundBox& bbox) const
{
	float *f=(float *)&bbox.min.x;

	for( int i=0;i<5;i++ )
		if (planes[i].x*f[bboxindx[i][0]]+
			planes[i].y*f[bboxindx[i][1]]+
			planes[i].z*f[bboxindx[i][2]]-planes[i].w<0)
			return 0;
	return 1;
}

void flyFrustum::draw() const 
{
	glColor4ub(255,255,255,255);
	g_flytexcache->sel_tex(-1);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_LINES);
	glVertex3fv(&verts[0].x);
	glVertex3fv(&verts[1].x);
	glVertex3fv(&verts[0].x);
	glVertex3fv(&verts[2].x);
	glVertex3fv(&verts[0].x);
	glVertex3fv(&verts[3].x);
	glVertex3fv(&verts[0].x);
	glVertex3fv(&verts[4].x);
	glEnd();
}
