#include "../Fly3D.h"

void flyLocalSystem::align_z(const flyVector& z)
{
	if (fabs(z.z)>=0.99f)
		{
		Y.x=-z.y*z.x;
		Y.y=1-z.y*z.y;
		Y.z=-z.y*z.z;

		Y.normalize();
		X.cross(Y,z);
		}
	else 
		{
		Y.x=-z.z*z.x;
		Y.y=-z.z*z.y;
		Y.z=1-z.z*z.z;

		Y.normalize();
		X.cross(Y,z);
		}

	Z=z;

	update_mat();
}

void flyLocalSystem::update_mat()
{
	memset(&mat,0,sizeof(mat));

	mat.m[0][0]=X.x;
	mat.m[0][1]=X.y;
	mat.m[0][2]=X.z;

	mat.m[1][0]=Y.x;
	mat.m[1][1]=Y.y;
	mat.m[1][2]=Y.z;

	mat.m[2][0]=Z.x;
	mat.m[2][1]=Z.y;
	mat.m[2][2]=Z.z;
	
	mat.m[3][3]=1;

	int i,j;
	for( i=0;i<4;i++ )
		for( j=0;j<4;j++ )
			mat_t.m[i][j]=mat.m[j][i];

}

void flyLocalSystem::rotate(const flyVector &v, const flyVector &u, float maxang)
{
	float f=FLY_VECDOT(u,v);
	if(f<0.99999f)
	{
		f=(float)acos(f)*FLY_PIUNDER180;

		if(f>maxang)
			f=maxang;
		else if(f<-maxang)
			f=-maxang;

		flyVector c;
		c.cross(v, u);
		c.normalize();
		rotate(f, c);
	}
}

void flyLocalSystem::rotate(const flyVector& rot)
{
	static flyMatrix m;

	m.set_rotation(rot.x,X);
	m.rotate(rot.y,Y);
	m.rotate(rot.z,Z);

	X=X*m;
	Y=Y*m;
	Z=Z*m;
	
	update_mat();
}

void flyLocalSystem::rotate(float ang,const flyVector& v)
{
	static flyMatrix m;

	m.set_rotation(ang,v);

	X=X*m;
	Y=Y*m;
	Z=Z*m;
	
	update_mat();
}


