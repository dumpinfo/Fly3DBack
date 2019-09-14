#include "../Fly3D.h"
#include "cpuid.h"

flySkeletonMesh::flySkeletonMesh(const flySkeletonMesh& in) :
	flyMesh(in),
	nkeys(in.nkeys),
	nbones(in.nbones),
	root(in.root)
{ 
	int i,j=0;
	nvweights=new int[nv];
	for( i=0;i<nv;i++ )
		j+=(nvweights[i]=in.nvweights[i]);

	vindex=new int[j];
	vweight=(flyVector *)aligned_alloc(sizeof(flyVector)*j);
	for( i=0;i<j;i++ )
	{
		vindex[i]=in.vindex[i];
		vweight[i]=in.vweight[i];
	}
	
	parent=new int[nbones];
	m=(flyMatrix *)aligned_alloc(sizeof(flyMatrix)*nbones);
	for( i=0;i<nbones;i++ )
	{
		parent[i]=in.parent[i];
		strcpy(bonenames[i],in.bonenames[i]);
		m[i]=in.m[i];
	}

	j=nbones*nkeys;

	tm=(flyMatrix *)aligned_alloc(sizeof(flyMatrix)*j);
	for( i=0;i<j;i++ )
		tm[i]=in.tm[i];

	for( i=0;i<nanims;i++ )
	{
		animkeys[i]=in.animkeys[i];
		animkeyspos[i]=in.animkeyspos[i];
		strcpy(animnames[i],in.animnames[i]);
	}
}

void flySkeletonMesh::reset()
{
	delete[] nvweights;
	delete[] vindex;
	aligned_free(vweight);
	aligned_free(tm);
	aligned_free(m);
	delete[] parent;
	parent=0;
	nbones=0;
	nvweights=0;
	vindex=0;
	vweight=0;
	tm=0;
	m=0;
	nkeys = 0;

	flyMesh::reset();
	type=FLY_TYPE_SKELETONMESH;
}

void flySkeletonMesh::operator=(const flySkeletonMesh& in)
{
	reset();
	
	flyMesh::operator=(in);

	nkeys=in.nkeys;
	nbones=in.nbones;
	root=in.root;

	int i,j=0;
	nvweights=new int[nv];
	for( i=0;i<nv;i++ )
		j+=(nvweights[i]=in.nvweights[i]);

	vindex=new int[j];
	vweight=(flyVector *)aligned_alloc(sizeof(flyVector)*j);
	for( i=0;i<j;i++ )
	{
		vindex[i]=in.vindex[i];
		vweight[i]=in.vweight[i];
	}
	
	parent=new int[nbones];
	m=(flyMatrix *)aligned_alloc(sizeof(flyMatrix)*nbones);
	for( i=0;i<nbones;i++ )
	{
		parent[i]=in.parent[i];
		strcpy(bonenames[i],in.bonenames[i]);
		m[i]=in.m[i];
	}

	j=nbones*nkeys;
	tm=(flyMatrix *)aligned_alloc(sizeof(flyMatrix)*j);
	for( i=0;i<j;i++ )
		tm[i]=in.tm[i];

	for( i=0;i<nanims;i++ )
	{
		animkeys[i]=in.animkeys[i];
		animkeyspos[i]=in.animkeyspos[i];
		strcpy(animnames[i],in.animnames[i]);
	}
}

int flySkeletonMesh::get_key_factor(int anim,float& key_factor)
{
	int key=(int)(key_factor*animkeys[anim]);
	if (key==animkeys[anim])
		{ key=0; key_factor=0.0f; }
	
	float f=1.0f/animkeys[anim];
	key_factor=(key_factor-key*f)/f;

	return key;
}

void flySkeletonMesh::set_skeleton(int anim, int key)
{
	memcpy(m,&tm[animkeyspos[anim]*nbones+key],sizeof(flyMatrix)*nbones);
}

void flySkeletonMesh::set_skeleton(int anim, float key_factor)
{
	int i,key1,key2;

	key1=get_key_factor(anim,key_factor);

	if (key1==animkeys[anim]-1)
		key2=0;
	else key2=key1+1;

	float f=1.0f-key_factor;

	key1=(animkeyspos[anim]+key1)*nbones;
	key2=(animkeyspos[anim]+key2)*nbones;

	for( i=0;i<nbones;i++ )
		m[i].lerp(tm[key1+i],tm[key2+i],key_factor);
}

void flySkeletonMesh::set_skeleton(int anim1, float key_factor1, int anim2, float key_factor2, float blend)
{
	int i,key1,key2,key3,key4;

	key1=get_key_factor(anim1,key_factor1);
	key3=get_key_factor(anim2,key_factor2);

	if (key1==animkeys[anim1]-1)
		key2=0;
	else key2=key1+1;
	if (key3==animkeys[anim2]-1)
		key4=0;
	else key4=key3+1;

	key1=(animkeyspos[anim1]+key1)*nbones;
	key2=(animkeyspos[anim1]+key2)*nbones;
	key3=(animkeyspos[anim2]+key3)*nbones;
	key4=(animkeyspos[anim2]+key4)*nbones;

	static flyMatrix m1,m2;
	m=(flyMatrix *)aligned_alloc(sizeof(flyMatrix)*nbones);
	for( i=0;i<nbones;i++ )
	{
		m1.lerp(tm[key1+i],tm[key2+i],key_factor1);
		m2.lerp(tm[key3+i],tm[key4+i],key_factor2);
		m[i].lerp(m1,m2,blend);
	}
}

int flySkeletonMesh::save(const char *file) const 
{
	return 0;
}

int flySkeletonMesh::load(const char *file)
{
	int i,j,k,l;
	flyFile fp;
		
	// open file
	if (fp.open(file)==0)
		return 0;

	// read filetype and version
	fp.read(&type,sizeof(int));
	fp.read(&j,sizeof(int));
	if (type!=FLY_FILEID_K3D || j!=3)
	{
		fp.close();
		return 0;
	}

	// resets all internal data
	reset();

	// read header
	fp.read(&pivotpos,sizeof(float)*3);
	fp.read(&nf,sizeof(int));
	fp.read(&nv,sizeof(int));
	fp.read(&nbones,sizeof(int));
	
	// create base flyMesh data
	localvert=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nv);
	localfaces=new flyFace[nf];
	faces=new flyFace *[nf];
	parent=new int[nbones];

	// load base flyMesh color and texture data
	for( i=0;i<nv;i++ )
		fp.read(&localvert[i].texcoord.x,2*sizeof(float));
	for( i=0;i<nv;i++ )
		fp.read(&localvert[i].color,sizeof(int));

	// create a load flySkeletonMesh weights info 
	int totweights=0;
	nvweights=new int[nv];
	fp.read(nvweights,nv*sizeof(int));
	for( i=0;i<nv;i++ )
		totweights+=nvweights[i];

	// create flySkeletonMesh data
	vindex=new int[totweights];
	vweight=(flyVector *)aligned_alloc(sizeof(flyVector)*totweights);

	// load flySkeletonMesh vert data
	fp.read(vindex,totweights*sizeof(int));
	fp.read(vweight,totweights*sizeof(flyVector));

	// load face data
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

	// load parent node info and node names
	root=-1;
	fp.read(parent,sizeof(int)*nbones);
	for( i=0;i<nbones;i++ )
	{
		fp.read(bonenames[i],FLY_MAX_MESHSTRLEN);
		if (parent[i]==-1)
			root=i;
	}

	// load anims info
	fp.read(&nanims,sizeof(int));
	fp.read(animkeys,nanims*sizeof(int));
	fp.read(animnames,nanims*FLY_MAX_MESHSTRLEN);

	// sum anim keys
	for(i = 0; i < nanims; i++)
	{
		animkeyspos[i] = nkeys;
		nkeys += animkeys[i];
	}

	// alloc anims tm
	tm=(flyMatrix *)aligned_alloc(sizeof(flyMatrix)*nbones*nkeys);
	m=(flyMatrix *)aligned_alloc(sizeof(flyMatrix)*nbones);

	// load flySkeletonMesh tm data
	k = 0;
	for( j = 0; j<nkeys; j++ )
		for( i=0;i<nbones;i++,k++ )
		{
			tm[k].load_identity();
			for( l=0;l<4;l++ )
				fp.read(&tm[k].m[l][0],3*sizeof(float));
		}

	// close file
	fp.close();

	// compute normals
	localen=(flyVector *)aligned_alloc(sizeof(flyVector)*nen);
	j=0;
	for( i=0;i<nf;i++ )
	{
		localfaces[i].sortkey=(localfaces[i].sh<<16);
		localfaces[i].en=&localen[j];
		j+=localfaces[i].ntriface*4;
	}

	if (root!=-1)
	{
		flyMatrix m1,m2;

		int b;
		k=0;
		for( i=0;i<nanims;i++ )
		{
			m1=tm[k*nbones+root];
			m2.load_identity();
			for( j=0;j<3;j++ )
				for( b=0;b<3;b++ )
					m2.m[j][b]=m1.m[b][j];

			m2.rotate(90,flyVector(0,0,1));
			m2.rotate(-90,flyVector(0,1,0));
			
			for( j=0;j<animkeys[i];j++ )
				for( b=0;b<nbones;b++ )
				{
					tm[(k+j)*nbones+b].m[3][0]-=m1.m[3][0];
					tm[(k+j)*nbones+b].m[3][1]-=m1.m[3][1];
					tm[(k+j)*nbones+b].m[3][2]-=m1.m[3][2];
					tm[(k+j)*nbones+b]=tm[(k+j)*nbones+b]*m2;
				}

			k+=j;
		}
	}
	
	set_skeleton(0,0);
	build_mesh();
	color.vec(1,1,1,1);
	compute_normals();

	return 1;
}

int flySkeletonMesh::get_bone(const char *name) const 
{
	for( int i=0;i<nbones;i++ )
		if (!stricmp(bonenames[i],name))
			return i;
	return -1;
}

void flySkeletonMesh::build_mesh()
{
	int i,j;
	int p=0;
#ifdef INTEL_SIMD
	if (g_processor_features&_CPU_FEATURE_SSE && g_flyengine->sse)
	{
		flyVertex *v1=localvert;
		flyVector *v2=vweight;
		flyMatrix *m1=m;
		int *i1=vindex;
		flyVector f1(0);

		// for all vertices
		for( i=0;i<nv;i++ )
		{
			j=nvweights[i];
			__asm
			{
				// xmm0=(0,0,0,0)
				movups xmm0,f1

				// ebx=&vindex[p]
				mov ebx,p
				add ebx,i1
				
				// initialize loop register
				mov ecx,0
				
			L1:			
				// compute matrix memory pos 
				// for vindex[p]
				mov eax,[ebx]
				sal eax,0x6
				add eax,m1

				// load matrix m[vindex[p]] into xmm4-7
				movaps xmm4,[eax]
				movaps xmm5,[eax+16]
				movaps xmm6,[eax+32]
				movaps xmm7,[eax+48]

				// xmm3=v2
				mov eax,v2
				movaps xmm3,[eax]

				// multiply first matrix line
				// and accumulate into xmm0
				movaps xmm1,xmm3
				shufps xmm1,xmm1,0x00
				mulps xmm1,xmm4
				addps xmm0,xmm1

				// multiply second matrix line
				// and accumulate into xmm0
				movaps xmm1,xmm3
				shufps xmm1,xmm1,0x55
				mulps xmm1,xmm5
				addps xmm0,xmm1

				// multiply third matrix line
				// and accumulate into xmm0
				movaps xmm1,xmm3
				shufps xmm1,xmm1,0xAA
				mulps xmm1,xmm6
				addps xmm0,xmm1
				
				// multiply fouth matrix line
				// and accumulate into xmm0
				movaps xmm1,xmm3
				shufps xmm1,xmm1,0xFF
				mulps xmm1,xmm7
				addps xmm0,xmm1

				// increment loop registers
				add v2,0x10
				add ebx,0x4
				
				// loop until all matrices influencing 
				// this vertex are processed
				inc ecx
				cmp ecx,j
				jl L1
				
				// v1=xmm0, v1++
				mov eax,v1
				movaps [eax],xmm0
				add v1,0x40

				// store current p
				sub ebx,i1
				mov p,ebx
			}
		}
	}
	else
#endif
	for( i=0;i<nv;i++ )
	{
		localvert[i].null();
		for( j=0;j<nvweights[i];j++,p++ )
			//	vertex = (matrix * vector) * float
			localvert[i] += (m[vindex[p]]*vweight[p])*vweight[p].w;
	}
}

void flySkeletonMesh::draw_skeleton() const
{
	g_flytexcache->sel_tex(-1);
	glColor4f(1,1,1,1);

	glBegin(GL_LINES);
	for( int i=0;i<nbones;i++ )
	if (parent[i]!=-1)
	{
		glVertex3fv(&m[i].m[3][0]);
		glVertex3fv(&m[parent[i]].m[3][0]);
	}
	glEnd();
}

