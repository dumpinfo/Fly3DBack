#include "../Fly3D.h"

flyBezierCurve::flyBezierCurve() : 
	np(0), ns(0), nd(0), p(0), pivot(0,0,0) 
{ 
	type=FLY_TYPE_BEZIERCURVE; 
}

flyBezierCurve::flyBezierCurve(const flyBezierCurve& in) : 
	flyBaseObject(in),
	p(new float[in.np*in.nd]),
	np(in.np),
	ns(in.ns),
	nd(in.nd),
	pivot(in.pivot) 
{ 
	for(int i=0; i<np*nd; i++) 
		p[i] = in.p[i];
}

flyBezierCurve::~flyBezierCurve() 
{ 
	reset(); 
}

void flyBezierCurve::operator=(const flyBezierCurve& in) 
{
	flyBaseObject::operator=(in);

	delete[] p;

	np = in.np;
	ns = in.ns;
	nd = in.nd;
	pivot = in.pivot;

	p = new float[np*nd];
	for(int i=0; i<np*nd; i++)  
		p[i] = in.p[i];
}

float flyBezierCurve::distance(const float *p,const float *p1,const float *p2) const
{
	// returns distance from point p to line defined by p1,p2

	float dot=0,len1=0,len2=0;
	int i;

	// alloc two vectors
	float *v1=new float [nd];
	float *v2=new float [nd];
	
	// compute length from (p1,p) and (p1,p2)
	for( i=0;i<nd;i++ )
	{
		v1[i]= p[i]-p1[i];
		v2[i]=p2[i]-p1[i];
		len1+=v1[i]*v1[i];
		len2+=v2[i]*v2[i];
	}
	len1=(float)sqrt(len1);
	len2=(float)sqrt(len2);
	
	// normalize 
	if (len1>0)
		for( i=0;i<nd;i++ )
			v1[i]/=len1;
	if (len2>0)
		for( i=0;i<nd;i++ )
			v2[i]/=len2;
	
	// compute dot product
	for( i=0;i<nd;i++ )
		dot+=v1[i]*v2[i];

	// free vectors
	delete[] v1; 
	delete[] v2;

	// return distance
	return (float)sqrt(len1*len1*(1.0f-dot*dot));
}

int flyBezierCurve::adaptative_subdiv(float maxerror,float *points,int maxpoints) const
{
	if (maxpoints<2) return 0;

	int npoints,i,j;
	float tmp;

	// start points list with two points (the curve edges)
	points[0]=0.0f;
	points[1]=1.0f;
	npoints=2;

	// subdiv curve recursevily
	subdiv(0,1,&p[0],&p[nd*(np-1)],points,npoints,maxerror,maxpoints);

	// sort points
	for( i=1;i<npoints;i++ )
		for( j=i+1;j<npoints;j++ )
			if (points[j]<points[i])
			{
				tmp=points[i];
				points[i]=points[j];
				points[j]=tmp;
			}

	// sort number of points
	return npoints;
}

void flyBezierCurve::subdiv(float u1,float u2,const float *p1,const float *p2,float *points,int& npoints,float maxerror,int maxpoints) const
{
	// if points list is not full
	if (npoints<maxpoints)
	{
	// alloc a new point
	float *p=new float[nd];
	
	// compute segment midpoint coordinate
	float u=(u1+u2)*0.5f;

	// evaluate curve at the segment midpoint
	evaluate(u,p);

	// if error is bigger then maxerror
	if (distance(p,p1,p2)>maxerror)
		{
		// add point to points list
		points[npoints++]=u;
		// subdiv each segment recursevily
		subdiv(u1,u,p1,p,points,npoints,maxerror,maxpoints);
		subdiv(u,u2,p,p2,points,npoints,maxerror,maxpoints);
		}
	
	// delete allocated point
	delete[] p;
	}
}

void flyBezierCurve::reset()
{
	delete[] p;
	p=0;
	ns=0;
	np=0;
}

void flyBezierCurve::set_dim(int ndim)
{
	reset();
	nd=ndim;
}

void flyBezierCurve::add_point(const float *f)
{
	// adds a new point to the curve
	// f must point to nd floats

	if (nd==0) return;
    float *t=new float[(np+1)*nd];
    if (p!=0)
        {
         memcpy(t,p,sizeof(float)*np*nd);
		 delete[] p;
        }
    p=t;
	memcpy(&p[np*nd],f,sizeof(float)*nd);
	np++;
    if (((np-4)%3)==0 && np>1)
       ns++;
}

void flyBezierCurve::evaluate_tangent(float u,float *f) const 
{	
	// evaluate the curve tangent at position u
	// f must point to nd floats

	if (p==0) return;

	static float B[4];
	float u1,u2;
	int s,a,b,c;

	if (u>1.0f) u=1.0f;
	if (u<0.0f) u=0.0f;

	if (u==1.0f)
		s=np-4;
	else s=(int)(u*(np-1))/3*3;
	u1=(u-(float)s/(np-1))*ns;

	u2=u1*u1;
	B[0] =-3.0f +  6.0f*u1 - 3.0f*u2;
	B[1] = 3.0f - 12.0f*u1 + 9.0f*u2;
	B[2] = 6.0f*u1 - 9.0f*u2;
	B[3] = 3.0f*u2;

	memset(f,0,sizeof(float)*nd);

	c=s*nd;
	for( a=0;a<4;a++ )
		for( b=0;b<nd;b++ )
			f[b]+=p[c++]*B[a];
}

void flyBezierCurve::evaluate(float u,float *f) const 
{
	// evaluate the curve point at position u
	// f must point to nd floats

	if (p==0) return;

	if (u>1.0f) u=1.0f;
	if (u<0.0f) u=0.0f;

	static float B[4];
	float u1,u2;
	int s,a,b,c;

	if (u==1.0f)
		s=np-4;
	else s=(int)(u*(np-1))/3*3;
	u1=(u-(float)s/(np-1))*ns;

	u2=1.0f-u1;
	B[0]=u2*u2*u2;
	B[1]=3.0f*u1*u2*u2;
	B[2]=3.0f*u1*u1*u2;
	B[3]=u1*u1*u1;

	memset(f,0,sizeof(float)*nd);

	c=s*nd;
	for( a=0;a<4;a++ )
		for( b=0;b<nd;b++ )
			f[b]+=p[c++]*B[a];
}

int flyBezierCurve::load_bez(const char *file)
{
	// loads the curve from a .bez text file

	flyFile fp;
	if (!fp.open(file))
		return 0;

	reset();
	int i,j,n,d;
	float *f;
	
	pivot.x=fp.get_float();
	pivot.y=fp.get_float();
	pivot.z=fp.get_float();
	d=fp.get_int();
	n=fp.get_int();
	set_dim(d);
	f=new float[d];
	for( i=0;i<n;i++ )
		{
		for( j=0;j<d;j++ )
			f[j]=fp.get_float();
		add_point(f);
		}
	delete[] f;
	fp.close();
	return 1;
}

float flyBezierCurve::length() const 
{
	if (ns==0 || nd==0) return 0;

	int i,j=ns*8,k,vv=1;
	float *v[2],len=0.0f,f;
	v[0]=new float[nd];
	v[1]=new float[nd];
	evaluate(0.0f,v[0]);
	for( i=1;i<j;i++ )
	{
		evaluate((float)i/(j-1),v[vv]);
		f=0.0f;
		for( k=0;k<nd;k++ )
			f+=(v[0][k]-v[1][k])*(v[0][k]-v[1][k]);
		len+=(float)sqrt(f);
		vv=!vv;
	}

	delete[] v[0];
	delete[] v[1];

	return len;
}

flyBezierPatch::flyBezierPatch() : 
	p(0), surf(0) 
{ 
	reset(); 
}

flyBezierPatch::~flyBezierPatch()
{ 
	reset(); 
}

flyBezierPatch::flyBezierPatch(const flyBezierPatch& in) :
	mode(in.mode),
	np(in.np),
	npu(in.npu),npv(in.npv),
	nsu(in.nsu),nsv(in.nsv),
	levelu(in.levelu),levelv(in.levelv),
	numstrips(in.numstrips),
	dropu(in.dropu),dropv(in.dropv),
	dropu_inc(in.dropu_inc),dropv_inc(in.dropv_inc) 
{
	int i;
	p=(flyVertex *)aligned_alloc(sizeof(flyVertex)*np);
	for( i=0;i<np;i++ )
		p[i] = in.p[i];
	surf=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nvertu*nvertv);
	for( i=0;i<nvertu*nvertv;i++ )
		surf[i] = in.surf[i];
}

void flyBezierPatch::operator=(const flyBezierPatch& in) 
{ 
	np = in.np;
	npu = in.npu; npv = in.npv;
	nsu = in.nsu; nsv = in.nsv;
	levelu = in.levelu; levelv = in.levelv;
	numstrips = in.numstrips;
	dropu = in.dropu; dropv = in.dropv;
	dropu_inc = in.dropu_inc; dropv_inc = in.dropv_inc;

	aligned_free(p);
	aligned_free(surf);

	int i;
	p=(flyVertex *)aligned_alloc(sizeof(flyVertex)*np);
	for( i=0;i<np;i++ )
		p[i] = in.p[i];
	surf=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nvertu*nvertv);
	for( i=0;i<nvertu*nvertv;i++ )
		surf[i] = in.surf[i];
}

void flyBezierPatch::build_cp(int nvu,int nvv,const flyVertex *v)
{
	mode=3;
	npu=nvu;
	npv=nvv;
	np=npu*npv;
	nsu=(npu-1)/2;
	nsv=(npv-1)/2;
	p=(flyVertex *)aligned_alloc(sizeof(flyVertex)*np);
	memcpy(p,v,sizeof(flyVertex)*np);
}

void flyBezierPatch::build_cp_loft(const flyBezierCurve *shape,const flyBezierCurve *path,float tileu,float tilev)
{
	reset();
	if (shape && path && shape->nd==3 && path->nd==3)
	{
	mode=4;
	npu=shape->np;
	npv=path->np;
	np=npu*npv;
	nsu=shape->ns;
	nsv=path->ns;
	p=(flyVertex *)aligned_alloc(sizeof(flyVertex)*np);
	flyVector v1,v2;
	flyLocalSystem ls;
	int i,j,k=0;
	ls.align_z(flyVector(0,0,1));
	for( j=0;j<npv;j++ )
		{
		path->evaluate_tangent((float)j/(npv-1),&v1.x);
		v1.normalize();
		ls.rotate(ls.Z,v1);
		v1=*((flyVector *)&path->p[j*3]);
		for( i=0;i<npu;i++ )
			{
			v2=*((flyVector *)&shape->p[i*3])*ls.mat;
			p[k].x=v1.x+v2.x;
			p[k].y=v1.y+v2.y;
			p[k].z=v1.z+v2.z;
			p[k].texcoord.x=(float)i/(npu-1)*tileu;
			p[k].texcoord.y=(float)j/(npv-1)*tilev;
			p[k].texcoord.z=(float)i/(npu-1);
			p[k].texcoord.w=(float)j/(npv-1);
			k++;
			}
		}
	}
}

void flyBezierPatch::evaluate(int evaltype,float u,float v,flyVector *dest) const 
{
	if (p==0) return;

	if (u>1.0f) u=1.0f;
	if (u<0.0f) u=0.0f;
	if (v>1.0f) v=1.0f;
	if (v<0.0f) v=0.0f;

	static float Bu[4],Bv[4],Bdu[4],Bdv[4];
	static flyVector point,textcoord,du,dv;
	static float u1,u2,v1,v2,f;
	static int i,j,k,su,sv;
	point.x=point.y=point.z=0.0f;
	textcoord.x=textcoord.y=textcoord.z=textcoord.w=0.0f;

	switch(mode)
	{
	case 3:
		if (FLY_FPBITS(u)==FLY_FPONEBITS)
			su=npu-3;
		else su=(int)(u*(npu-1))/2*2;
		u1=(u-(float)su/(npu-1))*nsu;

		if (FLY_FPBITS(v)==FLY_FPONEBITS)
			sv=npv-3;
		else sv=(int)(v*(npv-1))/2*2;
		v1=(v-(float)sv/(npv-1))*nsv;

		u2=1.0f-u1;
		Bu[0]=u2*u2;
		Bu[1]=2.0f*u1*u2;
		Bu[2]=u1*u1;

		v2=1.0f-v1;
		Bv[0]=v2*v2;
		Bv[1]=2.0f*v1*v2;
		Bv[2]=v1*v1;

		if (evaltype&FLY_PATCHEVAL_NORMAL)
			{
			du.x=du.y=du.z=0.0f;
			dv.x=dv.y=dv.z=0.0f;

			Bdu[0]= 2.0f*u1-2.0f;
			Bdu[1]= 2.0f-4.0f*u1;
			Bdu[2]= 2.0f*u1;

			Bdv[0]= 2.0f*v1-2.0f;
			Bdv[1]= 2.0f-4.0f*v1;
			Bdv[2]= 2.0f*v1;
			}
		break;
	case 4:
		if (FLY_FPBITS(u)==FLY_FPONEBITS)
			su=npu-4;
		else su=(int)(u*(npu-1))/3*3;
		u1=(u-(float)su/(npu-1))*nsu;

		if (FLY_FPBITS(v)==FLY_FPONEBITS)
			sv=npv-4;
		else sv=(int)(v*(npv-1))/3*3;
		v1=(v-(float)sv/(npv-1))*nsv;

		u2=1.0f-u1;
		Bu[0]=u2*u2*u2;
		Bu[1]=3.0f*u1*u2*u2;
		Bu[2]=3.0f*u1*u1*u2;
		Bu[3]=u1*u1*u1;

		v2=1.0f-v1;
		Bv[0]=v2*v2*v2;
		Bv[1]=3.0f*v1*v2*v2;
		Bv[2]=3.0f*v1*v1*v2;
		Bv[3]=v1*v1*v1;

		if (evaltype&FLY_PATCHEVAL_NORMAL)
			{
			du.vec(0,0,0);
			dv.vec(0,0,0);

			u2=u1*u1;
			Bdu[0]=-3.0f +  6.0f*u1 - 3.0f*u2;
			Bdu[1]= 3.0f - 12.0f*u1 + 9.0f*u2;
			Bdu[2]= 6.0f*u1 - 9.0f*u2;
			Bdu[3]= 3.0f*u2;

			v2=v1*v1;
			Bdv[0]=-3.0f +  6.0f*v1 - 3.0f*v2;
			Bdv[1]= 3.0f - 12.0f*v1 + 9.0f*v2;
			Bdv[2]= 6.0f*v1 - 9.0f*v2;
			Bdv[3]= 3.0f*v2;
			}
	}

	for( i=0;i<mode;i++ )
		for( j=0;j<mode;j++ )
		{
			k=(sv+j)*npu+su+i;
			f=Bu[i]*Bv[j];
			if (evaltype&FLY_PATCHEVAL_POINT)
				{
				point.x+=f*p[k].x;
				point.y+=f*p[k].y;
				point.z+=f*p[k].z;
				}
			if (evaltype&FLY_PATCHEVAL_TEXTCOORD)
				{
				textcoord.x+=f*p[k].texcoord.x;
				textcoord.y+=f*p[k].texcoord.y;
				textcoord.z+=f*p[k].texcoord.z;
				textcoord.w+=f*p[k].texcoord.w;
				}
			if (evaltype&FLY_PATCHEVAL_NORMAL)
				{
				du+=p[k]*(Bu[i]*Bdv[j]);
				dv+=p[k]*(Bdu[i]*Bv[j]);
				}
		}

	if (evaltype&FLY_PATCHEVAL_POINT)
		*(dest++)=point;
	if (evaltype&FLY_PATCHEVAL_TEXTCOORD)
		*(dest++)=textcoord;
	if (evaltype&FLY_PATCHEVAL_NORMAL)
		{
		dest->cross(dv,du);
		dest->normalize();
		}
}

void flyBezierPatch::reset()
{
	aligned_free(p);
	p=0;
	aligned_free(surf);
	surf=0;
	np=npu=npv=nsu=nsv=0;
	mode=0;
	levelu=0;
	levelv=0;
	nvertu=0;
	nvertv=0;
}

int find_subdiv_level(flyVector v0,flyVector v1,flyVector v2)
{
    int level;
    flyVector a, b, dist;
	
	float factor=g_flyengine->curveerr*g_flyengine->curveerr;

    for (level=0;level<8;level++)
    {
	a=(v0+v1)*0.5f;
	b=(v1+v2)*0.5f;
	v2=(a+b)*0.5f;

	dist=v2-v1;
	if (FLY_VECDOT(dist,dist)<factor)
	    break;
	v1=a;
    }

    return level;
}

void flyBezierPatch::build_surface()
{
	if (mode!=3 || p==0)
		return;
	int i,j,k,a;
	flyVector normal;

	levelu=0;
	levelv=0;
	k=0;
	for( j=0;j<npv;j++,k++ )
	for( i=0;i<nsu;i++,k+=2 )
	{
		a=find_subdiv_level(p[k],p[k+1],p[k+2]);
		if (a>levelu)
			levelu=a;
	}
	k=0;
	for( j=0;j<nsv;j++,k+=npu )
	for( i=0;i<npu;i++,k++ )
	{
		a=find_subdiv_level(p[k],p[k+npu],p[k+2*npu]);
		if (a>levelv)
			levelv=a;
	}

	nvertu=(1<<levelu)*nsu+1;
	nvertv=(1<<levelv)*nsv+1;
	if (nvertu<2) nvertu=2;
	if (nvertv<2) nvertv=2;

	aligned_free(surf);
	surf=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nvertu*nvertv);

	k=0;
	for( j=0;j<nvertv;j++ )
		for( i=0;i<nvertu;i++,k++ )
			evaluate(FLY_PATCHEVAL_POINT|FLY_PATCHEVAL_TEXTCOORD|FLY_PATCHEVAL_NORMAL,
				(float)i/(nvertu-1),(float)j/(nvertv-1),&surf[k]);
}

void flyBezierPatch::set_detail(const flyVector& pos,const flyVector& campos)
{
	int nleveldrop=(int)((pos-campos).length()*4/flyRender::s_farplane);

	if (levelu>=nleveldrop)
		dropu=nleveldrop;
	else dropu=levelu;
	if (levelv>=nleveldrop)
		dropv=nleveldrop;
	else dropv=levelv;

	numstrips=(1<<(levelv-dropv))*nsv;
	dropu=(1<<dropu);
	dropv=(1<<dropv);

	dropu_inc=dropu*(sizeof(flyVertex)/sizeof(float));
	dropv_inc=dropv*nvertu*(sizeof(flyVertex)/sizeof(float));
}

void flyBezierPatch::draw(int drawmode) const 
{
	if (surf==0)
		return;

	float *f;
	int i,j;
	
	glDisableClientState(GL_NORMAL_ARRAY);

	switch(drawmode)
	{
	case 0:
		for( j=0;j<numstrips;j++ )
			{
			f=((float *)surf)+j*dropv_inc;
			glBegin(GL_TRIANGLE_STRIP);
			for( i=0;i<nvertu;i+=dropu,f+=dropu_inc)
				{
				glTexCoord2fv(f+dropv_inc+4);
				glNormal3fv(f+dropv_inc+8);
				glVertex3fv(f+dropv_inc);
				glTexCoord2fv(f+4);
				glNormal3fv(f+8);
				glVertex3fv(f);
				}
			glEnd();
			}
		break;
	case 1:
		for( j=0;j<numstrips;j++ )
			{
			f=((float *)surf)+j*dropv_inc;
			glBegin(GL_TRIANGLE_STRIP);
			for( i=0;i<nvertu;i+=dropu,f+=dropu_inc)
				{
				glTexCoord2fv(f+6+dropv_inc);
				glNormal3fv(f+dropv_inc+8);
				glVertex3fv(f+dropv_inc);
				glTexCoord2fv(f+6);
				glNormal3fv(f+8);
				glVertex3fv(f);
				}
			glEnd();
			}
		break;
	case 2:
		for( j=0;j<numstrips;j++ )
			{
			f=((float *)surf)+j*dropv_inc;
			glBegin(GL_TRIANGLE_STRIP);
			for( i=0;i<nvertu;i+=dropu,f+=dropu_inc)
				{
				glMultiTexCoord2fvARB(GL_TEXTURE0_ARB,f+6+dropv_inc);
				glMultiTexCoord2fvARB(GL_TEXTURE1_ARB,f+4+dropv_inc);
				glNormal3fv(f+dropv_inc+8);
				glVertex3fv(f+dropv_inc);
				glMultiTexCoord2fvARB(GL_TEXTURE0_ARB,f+6);
				glMultiTexCoord2fvARB(GL_TEXTURE1_ARB,f+4);
				glNormal3fv(f+8);
				glVertex3fv(f);
				}
			glEnd();
			}
		break;
	}
	
	glEnableClientState(GL_NORMAL_ARRAY);
}
