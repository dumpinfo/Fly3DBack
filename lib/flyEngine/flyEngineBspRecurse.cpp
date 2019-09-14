#include "../Fly3D.h"

flyBspNode *flyEngine::find_node(const flyVector& p) const
{
	flyBspNode *n=bsp;
	float dist;

	while(n && n->leaf==-1)
	{
	dist=n->distance(p);
	if (dist>-0.01f)
		if (n->child[0])
			n=n->child[0];
		else 
			return 0;
	else if (n->child[1])
			n=n->child[1];
		else 
			return 0;
	}
	return n;
}

void flyEngine::send_bsp_message(const flyVector& p,float rad,int msg,int param,void *data,int elemtype,int pvsleaf)
{
	recurse_bsp(p,rad,elemtype,pvsleaf);
	for( int i=0;i<selobjs.num;i++ )
		selobjs[i]->message(p,rad,msg,param,data);
}

int flyEngine::collision_bsp(const flyVector& p1,const flyVector& p2,int elemtype)
{
	static flyBspNode *stack[64];
	int nstack=1,f;
	flyVector ip;
	stack[0]=bsp;

	flyVector rd=p2-p1;
	float d,dist=rd.length();
	rd/=dist;

	flyMesh *m;
	hitobj=0;
	hitmesh=0;
	
	float d1,d2;
	flyBspNode *n;
	cur_bsprecurse++;

	while(nstack)
	{
		n=stack[--nstack];

		if (n->leaf!=-1)
		{
			for( int e=0;e<n->elem.num;e++ )
				if ((elemtype==0 || n->elem[e]->type==elemtype) &&
					n->elem[e]->lastbsprecurse!=cur_bsprecurse &&
					excludecollision!=n->elem[e])
					{
						n->elem[e]->lastbsprecurse=cur_bsprecurse;

						m=n->elem[e]->ray_intersect(p1,rd,ip,d,f);
						if (m && d<dist)
							{
							dist=d;
							hitobj=n->elem[e];
							hitmesh=m;
							hitip=ip;
							hitdist=d;
							hitface=f&0xffff;
							hitsubface=f>>16;
							hitshader=m->faces[hitface]->sh;
							if (m->faces[hitface]->facetype==FLY_FACETYPE_LARGE_POLYGON)
								hitnormal=m->faces[hitface]->normal*n->elem[e]->mat;
							else hitnormal=m->faces[hitface]->en[hitsubface*4]*n->elem[e]->mat;
							}
					}
		}
		else
		{
			d1=n->distance(p1);
			d2=d1*n->distance(p2);

			if (FLY_FPSIGNBIT(d2))
				if (FLY_FPSIGNBIT(d1)==0)
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
			else 
				if (FLY_FPSIGNBIT(d1)==0)
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else 
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
		}
	}

	if (hitobj)
		return 1;

	return 0;
}

int flyEngine::collision_test(const flyVector& p1,const flyVector& p2,int elemtype)
{
	static flyBspNode *stack[64];

	flyVector rd=p2-p1;
	float dist=rd.length();
	if (dist<0.01f)
		return 0;
	rd*=1.0f/dist;
	hitobj=0;
	hitmesh=0;

	cur_bsprecurse++;
	
	flyBspNode *n;
	float d1,d2;
	int nstack=1;
	stack[0]=bsp;

	while(nstack)
	{
		n=stack[--nstack];

		if (n->leaf!=-1)
		{
			for( int e=0;e<n->elem.num;e++ )
				if ((elemtype==0 || n->elem[e]->type==elemtype) &&
					n->elem[e]->lastbsprecurse!=cur_bsprecurse &&
					excludecollision!=n->elem[e])
					{
						n->elem[e]->lastbsprecurse=cur_bsprecurse;
						if (n->elem[e]->ray_intersect_test(p1,rd,dist))
							return 1;
					}
		}
		else
		{
			d1=n->distance(p1);
			d2=d1*n->distance(p2);

			if (FLY_FPSIGNBIT(d2))
				if (FLY_FPSIGNBIT(d1)==0)
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
			else 
				if (FLY_FPSIGNBIT(d1)==0)
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else 
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
		}
	}

	return 0;
}

void flyEngine::recurse_bsp(const flyVector& p,float rad,int elemtype,int pvsleaf)
{
	static flyBspNode *stack[64];
	float d;
	int nstack=1;
	stack[0]=bsp;
	
	flyBspNode *n;
	cur_bsprecurse++;

	selnodes.clear();
	selobjs.clear();
	while(nstack)
	{
		n=stack[--nstack];

		if (n->leaf!=-1)
		{
			if (pvsleaf==-1 || pvsoff || 
				FLY_PVS_TEST(pvsleaf,n->leaf))
			{
				selnodes.add(n);
				if (elemtype!=-1)
				{
				for( int e=0;e<n->elem.num;e++ )
					if ((elemtype==0 || n->elem[e]->type==elemtype) &&
						n->elem[e]->lastbsprecurse!=cur_bsprecurse)
						{
							n->elem[e]->lastbsprecurse=cur_bsprecurse;
							selobjs.add(n->elem[e]);
						}
				}
			}
		}
		else
		{
			d=n->distance(p);

			if (fabs(d)<rad)
				{
				if (FLY_FPSIGNBIT(d)==0)
					{
					if (n->child[1])
						stack[nstack++]=n->child[1];
					if (n->child[0])
						stack[nstack++]=n->child[0];
					}
				else
					{
					if (n->child[0])
						stack[nstack++]=n->child[0];
					if (n->child[1])
						stack[nstack++]=n->child[1];
					}
				}
			else 
				if (FLY_FPSIGNBIT(d)==0)
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else 
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
		}
	}
}

void flyEngine::recurse_bsp(const flyVector& p1,const flyVector& p2,int elemtype,int pvsleaf)
{
	static flyBspNode *stack[64];
	float d1,d2;
	int nstack=1;
	stack[0]=bsp;
	
	flyBspNode *n;
	cur_bsprecurse++;

	selnodes.clear();
	selobjs.clear();
	while(nstack)
	{
		n=stack[--nstack];

		if (n->leaf!=-1)
		{
			if (pvsleaf==-1 || pvsoff ||
				FLY_PVS_TEST(pvsleaf,n->leaf))
			{
				selnodes.add(n);
				if (elemtype!=-1)
				{
				for( int e=0;e<n->elem.num;e++ )
					if ((elemtype==0 || n->elem[e]->type==elemtype) &&
						n->elem[e]->lastbsprecurse!=cur_bsprecurse)
						{
							n->elem[e]->lastbsprecurse=cur_bsprecurse;
							selobjs.add(n->elem[e]);
						}
				}
			}
		}
		else
		{
			d1=n->distance(p1);
			d2=d1*n->distance(p2);

			if (FLY_FPSIGNBIT(d2))
				if (FLY_FPSIGNBIT(d1)==0)
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
			else 
				if (FLY_FPSIGNBIT(d1)==0)
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else 
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
		}
	}
}

void flyEngine::recurse_bsp(const flyVector *p,int np,int elemtype,int pvsleaf)
{
	static flyBspNode *stack[64];

	float x1,x2;
	int i,nstack=1;
	stack[0]=bsp;
	
	flyBspNode *n;
	cur_bsprecurse++;

	selnodes.clear();
	selobjs.clear();
	while(nstack)
	{
		n=stack[--nstack];

		if (n->leaf!=-1)
		{
			if (pvsoff || pvsleaf==-1 || 
				FLY_PVS_TEST(pvsleaf,n->leaf))
			{
				selnodes.add(n);
				if (elemtype!=-1)
				{
				for( int e=0;e<n->elem.num;e++ )
					if ((elemtype==0 || n->elem[e]->type==elemtype) &&
						n->elem[e]->lastbsprecurse!=cur_bsprecurse)
						{
							n->elem[e]->lastbsprecurse=cur_bsprecurse;
							selobjs.add(n->elem[e]);
						}
				}
			}
		}
		else
		{
			x1=n->distance(p[0]);
			for( i=1;i<np;i++ )
				{
					x2=n->distance(p[i]);
					if (x1*x2<0)
						break;
				}
			if (i!=np)
				if (FLY_FPSIGNBIT(x1)==0)
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
			else 
				if (FLY_FPSIGNBIT(x1)==0)
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else 
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
		}
	}
}

void flyEngine::recurse_bsp_box(const flyVector& min,const flyVector& max,int elemtype,int pvsleaf)
{
	static flyBspNode *stack[64];

	float f;
	int nstack=1;
	stack[0]=bsp;
	
	flyBspNode *n;
	cur_bsprecurse++;

	selnodes.clear();
	selobjs.clear();
	while(nstack)
	{
		n=stack[--nstack];

		if (n->leaf!=-1)
		{
			if (pvsoff || pvsleaf==-1 || 
				FLY_PVS_TEST(pvsleaf,n->leaf))
			{
				selnodes.add(n);
				if (elemtype!=-1)
				{
				for( int e=0;e<n->elem.num;e++ )
					if ((elemtype==0 || n->elem[e]->type==elemtype) &&
						n->elem[e]->lastbsprecurse!=cur_bsprecurse)
						{
							n->elem[e]->lastbsprecurse=cur_bsprecurse;
							selobjs.add(n->elem[e]);
						}
				}
			}
		}
		else
		{
			f=n->distance(min);
			if (f*(n->normal.x*max.x+n->normal.y*max.y+n->normal.z*max.z-n->d0)>0 &&
				f*(n->normal.x*max.x+n->normal.y*min.y+n->normal.z*min.z-n->d0)>0 &&
				f*(n->normal.x*min.x+n->normal.y*max.y+n->normal.z*min.z-n->d0)>0 &&
				f*(n->normal.x*min.x+n->normal.y*min.y+n->normal.z*max.z-n->d0)>0 &&
				f*(n->normal.x*max.x+n->normal.y*max.y+n->normal.z*min.z-n->d0)>0 &&
				f*(n->normal.x*min.x+n->normal.y*max.y+n->normal.z*max.z-n->d0)>0 &&
				f*(n->normal.x*max.x+n->normal.y*min.y+n->normal.z*max.z-n->d0)>0)
				if (FLY_FPSIGNBIT(f)==0)
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else 
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
			else 
				if (FLY_FPSIGNBIT(f)==0)
				{
					if (n->child[1])
						stack[nstack++]=n->child[1];
					if (n->child[0])
						stack[nstack++]=n->child[0];
				}
				else
				{
					if (n->child[0])
						stack[nstack++]=n->child[0];
					if (n->child[1])
						stack[nstack++]=n->child[1];
				}
		}
	}
}
