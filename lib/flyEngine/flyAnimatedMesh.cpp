#include "../Fly3D.h"
#include "cpuid.h"

#define FLY_MAX_MESHSTRLEN 32

flyAnimatedMesh::flyAnimatedMesh() :
	nkeys(0),
	key_verts(0)
{ 
	type=FLY_TYPE_VERTEXMESH; 
}

flyAnimatedMesh::~flyAnimatedMesh()
{ 
	reset(); 
}

flyAnimatedMesh::flyAnimatedMesh(const flyAnimatedMesh& in) :
	flyMesh(in),
	nkeys(in.nkeys)
{
	int i,j=nkeys*nv;
	key_verts = (flyVector *)aligned_alloc(sizeof(flyVector)*j);
	for( i=0;i<j;i++ )
		key_verts[i]=in.key_verts[i];
	for( i=0;i<nanims;i++ )
	{
		animkeys[i]=in.animkeys[i];
		animkeyspos[i]=in.animkeyspos[i];
		strcpy(animnames[i],in.animnames[i]);
	}
}

void flyAnimatedMesh::reset(void)
{
	aligned_free(key_verts);
	key_verts = 0;
	nkeys = 0;
	flyMesh::reset();
	type=FLY_TYPE_VERTEXMESH;
}
	
void flyAnimatedMesh::operator=(const flyAnimatedMesh& in)
{
	reset();
	
	flyMesh::operator=(in);
	
	nkeys=in.nkeys;
	int i,j=nkeys*nv;
	key_verts = (flyVector *)aligned_alloc(sizeof(flyVector)*j);
	for( i=0;i<j;i++ )
		key_verts[i]=in.key_verts[i];
	for( i=0;i<nanims;i++ )
	{
		animkeys[i]=in.animkeys[i];
		animkeyspos[i]=in.animkeyspos[i];
		strcpy(animnames[i],in.animnames[i]);
	}
}

int flyAnimatedMesh::save(const char *file) const 
{
	if (localvert==0 || localfaces==0)
		return 0;
	FILE *fp=fopen(file,"wb");
	if (fp==0)
		return 0;

	int i, j, k;
	i=FLY_FILEID_F3D;
	fwrite(&i,1,sizeof(int),fp);
	i=2;
	fwrite(&i,1,sizeof(int),fp);

	fwrite(&pivotpos.x,3,sizeof(float),fp);	// pivot	
	
	fwrite(&nf,1,sizeof(int),fp);	// numfaces
	fwrite(&nv,1,sizeof(int),fp);	// numvers

	for( i=0;i<nv;i++ )
		fwrite(&localvert[i].texcoord.x,2,sizeof(float),fp);
	for( i=0;i<nv;i++ )
		fwrite(&localvert[i].color,1,sizeof(int),fp);

	for( i=0;i<nf;i++ )
	{
		fwrite(&localfaces[i].ntriface,1,sizeof(int),fp);
		fwrite(localfaces[i].trivert,localfaces[i].ntrivert,sizeof(int),fp);
	}

	fwrite(&nanims,1,sizeof(int),fp);	// nanims
	fwrite(animkeys,nanims,sizeof(int),fp);	// nkeys
	fwrite(animnames,nanims,FLY_MAX_MESHSTRLEN,fp);	// animnames

	k = 0;
	for( j = 0; j != nkeys; j++)
		for( i=0;i<nv;i++ )
			fwrite(&key_verts[k++].x,3,sizeof(float),fp);

	fclose(fp);
	return 1;
}

int flyAnimatedMesh::load(const char *file)
{
	int i,j,k;
	flyFile fp;
		
	if (fp.open(file)==0)
		return 0;

	fp.read(&type,sizeof(int));
	fp.read(&j,sizeof(int));
	if (type!=FLY_FILEID_F3D || j!=2)
	{
		fp.close();
		return 0;
	}
	reset();

	fp.read(&pivotpos,sizeof(float)*3);

	fp.read(&nf,sizeof(int));
	fp.read(&nv,sizeof(int));
	
	localvert=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nv);
	localfaces=new flyFace[nf];
	faces=new flyFace *[nf];

	for( i=0;i<nv;i++ )
		fp.read(&localvert[i].texcoord.x,2*sizeof(float));
	for( i=0;i<nv;i++ )
		fp.read(&localvert[i].color,sizeof(int));

	nen=0;
	for( i=0;i<nf;i++ )
	{
		faces[i]=&localfaces[i];

		fp.read(&localfaces[i].ntriface,sizeof(int));
		localfaces[i].ntrivert=localfaces[i].ntriface*3;
		localfaces[i].trivert=new int[localfaces[i].ntrivert];
		fp.read(localfaces[i].trivert,localfaces[i].ntrivert*sizeof(int));

		localfaces[i].indx=i;
		localfaces[i].lm=-1;
		localfaces[i].nvert=nv;
		localfaces[i].vert=localvert;
		localfaces[i].vertindx=0;
		localfaces[i].sh=i;
		localfaces[i].facetype=FLY_FACETYPE_TRIANGLE_MESH;
		localfaces[i].patch=0;

		nen+=localfaces[i].ntriface*4;
	}

	fp.read(&nanims,sizeof(int));
	fp.read(animkeys,nanims*sizeof(int));
	fp.read(animnames,FLY_MAX_MESHSTRLEN*nanims);
	for(i = 0; i != nanims; i++)
	{
		animkeyspos[i] = nkeys;
		nkeys += animkeys[i];
	}

	key_verts = (flyVector *)aligned_alloc(sizeof(flyVector)*nkeys*nv);
	k = 0;
	for( j=0;j<nkeys;j++ )
		for( i=0;i<nv;i++ )
			fp.read(&key_verts[k++].x,3*sizeof(float));
		
	fp.close();

	localen = (flyVector *)aligned_alloc(sizeof(flyVector)*nen);
	j=0;
	for( i=0;i<nf;i++ )
	{
		localfaces[i].sortkey=(localfaces[i].sh<<16);
		localfaces[i].en=&localen[j];
		j+=localfaces[i].ntriface*4;
	}

	set_state(0,0);
	if (nanims==1 && animkeys[0]==1) // if static mesh (1 anim with 1 key)
		compute_normals(FLY_MESH_ALL);	// compute normals and build octree
	else
		compute_normals();	// just compute normals

	return 1;
}

void flyAnimatedMesh::set_state(int anim, int key)
{
	int i,j=animkeyspos[anim]*nv + key;
#ifdef INTEL_SIMD
	if (g_processor_features&_CPU_FEATURE_SSE && g_flyengine->sse)
	{
		flyVertex *v1=localvert;
		flyVector *v2=&key_verts[j];
		i=nv*0x40;
		
		__asm
		{
			mov ebx,0
			mov ecx,0
		L1:
			mov eax,v2
			movaps xmm0,[eax+ebx]
			
			mov eax,v1
			movaps [eax+ecx],xmm0

			add ebx,0x10
			add ecx,0x40

			cmp ecx,i
			jl L1
		}
	}
	else
#endif
	for( i=0;i<nv;i++ )
	{
		localvert[i].x=key_verts[j+i].x;
		localvert[i].y=key_verts[j+i].y;
		localvert[i].z=key_verts[j+i].z;
	}
}

void flyAnimatedMesh::set_state(int anim, float key_factor)
{
	int i,j,k;
	float s;
	flyVertex *v0;
	flyVector *v1,*v2;

	// compute local key factor between the two 
	// animation keys to be interpolated (key_factor)
	j=(int)(key_factor*animkeys[anim]);
	if (j==animkeys[anim])
		{ j=0; key_factor=0.0f; }
	s=1.0f/animkeys[anim];
	key_factor=(key_factor-j*s)/s;

	// find vertex pointers for the two 
	// animation keys to be interpolated (v1 and v2)
	v1=&key_verts[(animkeyspos[anim]+j)*nv];
	if (j==animkeys[anim]-1)
		k=0;
	else k=j+1;
	v2=&key_verts[(animkeyspos[anim]+k)*nv];

	v0=localvert;// output vertex array

	s=1.0f-key_factor; // compute 1-factors

#ifdef INTEL_SIMD
	if (g_processor_features&_CPU_FEATURE_SSE && g_flyengine->sse)
	{
		flyVector f1(s);
		flyVector f2(key_factor);
		i=nv*0x40;

		__asm
		{
			// xmm2=(s,s,s,s)
			// xmm3=(key_factor,key_factor,key_factor,key_factor)
			movups xmm2,f1
			movups xmm3,f2
	
			// init loop registers (ebx->flyVector, ecx->flyVertex)
			mov ebx,0
			mov ecx,0
		L1:
			// xmm0=v1[ebx]
			mov eax,v1
			movaps xmm0,[eax+ebx]

			// xmm0=v2[i]
			mov eax,v2
			movaps xmm1,[eax+ebx]

			// interpolate position to xmm0
			mulps xmm0,xmm2
			mulps xmm1,xmm3
			addps xmm0,xmm1

			// v0[i]=xmm0
			mov eax,v0
			movaps [eax+ecx],xmm0

			// increment loop registers
			add ebx,0x10
			add ecx,0x40

			// loop until all vertices are processed
			cmp	ecx, i
			jl L1
		}
	}
	else
#endif
	for( i=0;i<nv;i++)
	{
		v0->x = v1->x*s + v2->x*key_factor;
		v0->y = v1->y*s + v2->y*key_factor;
		v0->z = v1->z*s + v2->z*key_factor;
		v0++; v1++; v2++;
	}
}

void flyAnimatedMesh::set_state(int anim1, float key_factor1, int anim2, float key_factor2, float blend)
{
	int i,j,k;
	float s,t,w;
	flyVertex *v0;
	flyVector *v1,*v2,*v3,*v4;
	
	// compute local key factor between the first 
	// animation keys to be interpolated (key_factor1)
	j=(int)(key_factor1*animkeys[anim1]);
	if (j==animkeys[anim1])
		{ j=0; key_factor1=0.0f; }
	s=1.0f/animkeys[anim1];
	key_factor1=(key_factor1-j*s)/s;

	// find vertex pointers on the first animarion for 
	// the two keys to be interpolated (v1 and v2)
	v1=&key_verts[(animkeyspos[anim1]+j)*nv];
	if (j==animkeys[anim1]-1)
		k=0;
	else k=j+1;
	v2=&key_verts[(animkeyspos[anim1]+k)*nv];
			
	// compute local key factor between the second
	// animation keys to be interpolated (key_factor2)
	j=(int)(key_factor2*animkeys[anim2]);
	if (j==animkeys[anim2])
		{ j=0; key_factor2=0.0f; }
	s=1.0f/animkeys[anim2];
	key_factor2=(key_factor2-j*s)/s;

	// find vertex pointers on the second animarion for 
	// the two keys to be interpolated (v3 and v4)
	v3=&key_verts[(animkeyspos[anim2]+j)*nv];
	if (j==animkeys[anim2]-1)
		k=0;
	else k=j+1;
	v4=&key_verts[(animkeyspos[anim2]+k)*nv];

	v0 = localvert; // output vertex array

	s=1.0f-key_factor1;	// compute 1-factors
	t=1.0f-key_factor2;
	w=1.0f-blend;

#ifdef INTEL_SIMD
	if (g_processor_features&_CPU_FEATURE_SSE && g_flyengine->sse)
	{
		flyVector f1(s);
		flyVector f2(key_factor1);
		flyVector f3(t);
		flyVector f4(key_factor2);
		flyVector f5(w);
		flyVector f6(blend);
		i=nv*0x40;

		__asm
		{
			// xmm2=(s,s,s,s)
			// xmm3=(key_factor1,key_factor1,key_factor1,key_factor1)
			// xmm2=(t,t,t,t)
			// xmm3=(key_factor2,key_factor2,key_factor2,key_factor2)
			movups xmm4,f1
			movups xmm5,f2
			movups xmm6,f3
			movups xmm7,f4

			// init loop registers (ebx->flyVector, ecx->flyVertex)
			mov ebx,0
			mov ecx,0
		L1:
			// xmm0=v1[i]
			mov eax,v1
			movaps xmm0,[eax+ebx]
			
			// xmm1=v2[i]
			mov eax,v2
			movaps xmm1,[eax+ebx]

			// interpolate first anim to xmm0
			mulps xmm0,xmm4
			mulps xmm1,xmm5
			addps xmm0,xmm1

			// xmm2=v3[i]
			mov eax,v3
			movaps xmm2,[eax+ebx]

			// xmm3=v4[i]
			mov eax,v4
			movaps xmm3,[eax+ebx]
			
			// interpolate second anim to xmm2
			mulps xmm2,xmm6
			mulps xmm3,xmm7
			addps xmm2,xmm3
			
			// blend the two anims to xmm0
			movups xmm1,f5
			movups xmm3,f6
			mulps xmm0,xmm1
			mulps xmm2,xmm3
			addps xmm0,xmm2

			// v0[i]=xmm0
			mov eax,v0
			movaps [eax+ecx],xmm0

			// increment loop registers
			add ebx,0x10
			add ecx,0x40

			// loop until all vertices are processed
			cmp	ecx, i
			jl L1
		}
	}
	else
#endif
	for( i=0;i<nv;i++ )
	{
		v0->x=(v1->x*s+v2->x*key_factor1)*w+
			(v3->x*t+v4->x*key_factor2)*blend;
		v0->y=(v1->y*s+v2->y*key_factor1)*w+
			(v3->y*t+v4->y*key_factor2)*blend;
		v0->z=(v1->z*s+v2->z*key_factor1)*w+
			(v3->z*t+v4->z*key_factor2)*blend;
		v0++; v1++; v2++; v3++; v4++;
	}
}

void flyAnimatedMesh::set_state_blendcur(int anim, int key, float blend)
{
	flyVertex *v0=localvert;
	flyVector *v1=&key_verts[(animkeyspos[anim]+key)*nv];
#ifdef INTEL_SIMD
	if (g_processor_features&_CPU_FEATURE_SSE && g_flyengine->sse)
	{
		flyVector f1(1.0f-blend);
		flyVector f2(blend);
		int i=nv*0x40;

		__asm
		{
			movups xmm2,f1
			movups xmm3,f2

			mov ebx,0
			mov ecx,0
		L1:
			mov eax,v1
			movaps xmm1,[eax+ebx]

			mov eax,v0
			movaps xmm0,[eax+ecx]

			mulps xmm0,xmm2
			mulps xmm1,xmm3
			addps xmm0,xmm1

			movaps [eax+ecx],xmm0

			add ebx,0x10
			add ecx,0x40

			cmp	ecx, i
			jl L1
		}
	}
	else
#endif
	{
		float f=1.0f-blend;
		for( int i=0;i<nv;i++)
		{
			v0->x = v0->x*f + v1->x*blend;
			v0->y = v0->y*f + v1->y*blend;
			v0->z = v0->z*f + v1->z*blend;
			v0++; v1++;
		}
	}
}

void flyAnimatedMesh::set_state_blendcur(int anim, float key_factor, float blend)
{
	int i,j,k;
	float s,f;
	flyVertex *v0;
	flyVector *v1,*v2;

	j=(int)(key_factor*animkeys[anim]);
	if (j==animkeys[anim])
		{ j=0; key_factor=0.0f; }
	s=1.0f/animkeys[anim];
	key_factor=(key_factor-j*s)/s;

	v1=&key_verts[(animkeyspos[anim]+j)*nv];
	if (j==animkeys[anim]-1)
		k=0;
	else k=j+1;
	v2=&key_verts[(animkeyspos[anim]+k)*nv];

	v0=localvert;
	s=1.0f-key_factor;
	f=1.0f-blend;
#ifdef INTEL_SIMD
	if (g_processor_features&_CPU_FEATURE_SSE && g_flyengine->sse)
	{
		flyVector f1(f);
		flyVector f2(blend);
		flyVector f3(s);
		flyVector f4(key_factor);
		i=nv*0x40;

		__asm
		{
			movups xmm4,f1
			movups xmm5,f2
			movups xmm6,f3
			movups xmm7,f4
			
			mov ebx,0
			mov ecx,0
		L1:
			mov eax,v1
			movaps xmm1,[eax+ebx]
			
			add eax,v2
			movaps xmm2,[eax+ebx]
			
			mulps xmm1,xmm6
			mulps xmm2,xmm7
			addps xmm1,xmm2
			mulps xmm1,xmm5

			add eax,v0
			movaps xmm0,[eax+ecx]
			
			mulps xmm0,xmm4
			addps xmm0,xmm1

			movaps [eax+ecx],xmm0
			
			add ebx,0x10
			add ecx,0x40

			cmp	ecx, i
			jl L1
		}
	}
	else
#endif
	for( i=0;i<nv;i++)
	{
		v0->x = v0->x*f + (v1->x*s + v2->x*key_factor)*blend;
		v0->y = v0->y*f + (v1->y*s + v2->y*key_factor)*blend;
		v0->z = v0->z*f + (v1->z*s + v2->z*key_factor)*blend;
		v0++; v1++; v2++;
	}
}
