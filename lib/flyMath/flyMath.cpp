#include "..\flyMath.h"

void *aligned_alloc(int n)
{
	char *data=new char[n+20];
	unsigned t=(unsigned)data+4;
	t=((t&0xf)==0?0:16-(t&0xf));
	*((unsigned *)&data[t])=t;
	return &data[t+4];
}

void aligned_free(void *data)
{
	if (data==0)
		return;
	int t=*(((unsigned *)data)-1);
	char *d=(char *)data;
	d-=t+4;
	delete d;
}

int flyPlane::ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist)
{
	float x=FLY_VECDOT(normal,rd);
	if (FLY_FPSIGNBIT(x)==0)
		return 0;
	
	dist=(d0-FLY_VECDOT(normal,ro))/x;
	if (FLY_FPSIGNBIT(dist))
		return 0;

	ip=ro+rd*dist;

	return 1;
}

void flyMatrix::lerp(const flyMatrix& m1, const flyMatrix& m2,float t)
{
	flyQuaternion q1(m1),q2(m2),qt;
	qt.lerp(q1,q2,t);
	qt.get_mat(*this);
	m[3][0]=m1.m[3][0]*(1-t)+m2.m[3][0]*t;
	m[3][1]=m1.m[3][1]*(1-t)+m2.m[3][1]*t;
	m[3][2]=m1.m[3][2]*(1-t)+m2.m[3][2]*t;
}

void flyMatrix::rotate( float ang, const flyVector& dir )
{
	static flyMatrix m;
	m.set_rotation(ang,dir);
	*this=*this*m;
}

void flyMatrix::set_rotation( float ang, const flyVector& dir2 )
{
	flyVector dir(dir2);
	dir.normalize();

	ang*=-FLY_PIOVER180;

	float fCos=(float)cos( ang );
	float fSin=(float)sin( ang );

    m[0][0] = ( dir.x * dir.x ) * ( 1.0f - fCos ) + fCos;
    m[0][1] = ( dir.x * dir.y ) * ( 1.0f - fCos ) - (dir.z * fSin);
    m[0][2] = ( dir.x * dir.z ) * ( 1.0f - fCos ) + (dir.y * fSin);

    m[1][0] = ( dir.y * dir.x ) * ( 1.0f - fCos ) + (dir.z * fSin);
    m[1][1] = ( dir.y * dir.y ) * ( 1.0f - fCos ) + fCos ;
    m[1][2] = ( dir.y * dir.z ) * ( 1.0f - fCos ) - (dir.x * fSin);

    m[2][0] = ( dir.z * dir.x ) * ( 1.0f - fCos ) - (dir.y * fSin);
    m[2][1] = ( dir.z * dir.y ) * ( 1.0f - fCos ) + (dir.x * fSin);
    m[2][2] = ( dir.z * dir.z ) * ( 1.0f - fCos ) + fCos;
    
    m[0][3] = m[1][3] = m[2][3] = 0;
    m[3][0] = m[3][1] = m[3][2] = 0;
    m[3][3] = 1;
}

flyQuaternion::flyQuaternion(const flyMatrix &mat)
{

	float tr,s,q[4];
	int i,j,k;

	int nxt[3] = {1, 2, 0};

	tr = mat.m[0][0] + mat.m[1][1] + mat.m[2][2];

	// check the diagonal
	if (tr > 0.0) 
	{
		s = (float)sqrt(tr + 1.0f);
		w = s/2.0f;
		s = 0.5f/s;
		x = (mat.m[1][2] - mat.m[2][1]) * s;
		y = (mat.m[2][0] - mat.m[0][2]) * s;
		z = (mat.m[0][1] - mat.m[1][0]) * s;
	} 
	else 
	{		
		// diagonal is negative
    	i = 0;
		if (mat.m[1][1] > mat.m[0][0]) i = 1;
		if (mat.m[2][2] > mat.m[i][i]) i = 2;
		j = nxt[i];
		k = nxt[j];

		s=(float)sqrt((mat.m[i][i]-(mat.m[j][j] + mat.m[k][k])) + 1.0);
      
		q[i]=s*0.5f;
            
		if(s!=0.0f) s = 0.5f/s;

		q[3] = (mat.m[j][k] - mat.m[k][j]) * s;
		q[j] = (mat.m[i][j] + mat.m[j][i]) * s;
		q[k] = (mat.m[i][k] + mat.m[k][i]) * s;

		x = q[0];
		y = q[1];
		z = q[2];
		w = q[3];
	}
}

void flyQuaternion::get_mat(flyMatrix &mat) const 
{
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

	// calculate coefficients
	x2 = x + x; 
	y2 = y + y; 
	z2 = z + z;
	xx = x * x2;
	xy = x * y2;   
	xz = x * z2;
	yy = y * y2;   
	yz = y * z2;   
	zz = z * z2;
	wx = w * x2;   
	wy = w * y2;   
	wz = w * z2;

	mat.m[0][0] = 1.0f-(yy+zz); 	
	mat.m[1][0] = xy - wz;
	mat.m[2][0] = xz + wy;		
	mat.m[3][0] = 0.0;

	mat.m[0][1] = xy + wz;		
	mat.m[1][1] = 1.0f-(xx + zz);
	mat.m[2][1] = yz - wx;		
	mat.m[3][1] = 0.0;

	mat.m[0][2] = xz - wy;		
	mat.m[1][2] = yz + wx;
	mat.m[2][2] = 1.0f - (xx + yy);		
	mat.m[3][2] = 0.0;

	mat.m[0][3] = 0;			
	mat.m[1][3] = 0;
	mat.m[2][3] = 0;			
	mat.m[3][3] = 1;
}

flyQuaternion flyQuaternion::operator +(const flyQuaternion& q)
{
	flyQuaternion res;
	res.x=x+q.x;
	res.y=y+q.y;
	res.z=z+q.z;
	res.w=w+q.w;
	return res;
}

flyQuaternion flyQuaternion::operator *(const flyQuaternion &q)
{
	float A, B, C, D, E, F, G, H;
	flyQuaternion res;

	A = (q.w + q.x)*(w + x);
	B = (q.z - q.y)*(y - z);
	C = (q.w - q.x)*(y + z); 
	D = (q.y + q.z)*(w - x);
	E = (q.x + q.z)*(x + y);
	F = (q.x - q.z)*(x - y);
	G = (q.w + q.y)*(w - z);
	H = (q.w - q.y)*(w + z);

	res.w = B + (-E - F + G + H) /2;
	res.x = A - (E + F + G + H)/2; 
	res.y = C + (E - F + G - H)/2; 
	res.z = D + (E - F - G + H)/2;

	return res;
}

void flyQuaternion::normalize()
{
	float factor = 1.0f/(float)sqrt(x*x+y*y+z*z+w*w);

	x*=factor;
	y*=factor;
	z*=factor;
	w*=factor;
}

void flyQuaternion::get_rotate(float &angle, flyVector &axis) const
{
	angle=(float)acos(w)*2*FLY_PIUNDER180;

	float f=(float)sin(angle*FLY_PIOVER180*0.5f);

	axis.x=x/f;
	axis.y=x/f;
	axis.z=x/f;
}

void flyQuaternion::lerp(const flyQuaternion& q1, const flyQuaternion& q2,float t)
{
    float v;        // complement to t
    float o;        // complement to v (t)
    float theta;    // angle between q1 & q2
    float sin_t;    // sin(theta)
    float cos_t;    // cos(theta)
    float phi;      // spins added to theta
    int flip;       // flag for negating q2

    cos_t = q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2] + q1[3] * q2[3];

    if (cos_t < 0.0)
    {
		cos_t = -cos_t;
		flip = 1;
    }
    else 
		flip = 0;

    if (1.0 - cos_t < 1e-6)
    {
        v = 1.0f - t;
		o = t;
    }
    else
    {
        theta = (float)acos(cos_t);
        phi = theta;
        sin_t = (float)sin(theta);
        v = (float)sin(theta - t * phi) / sin_t;
        o = (float)sin(t * phi) / sin_t;
    }
    if (flip) o = -o;

    x = v * q1[0] + o * q2[0];
    y = v * q1[1] + o * q2[1];
    z = v * q1[2] + o * q2[2];
    w = v * q1[3] + o * q2[3];
}

