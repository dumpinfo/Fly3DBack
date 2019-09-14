#include "../Fly3D.h"
#include "cpuid.h"

flyMesh::flyMesh(const flyMesh& in) :
	flyBaseObject(in),
	nf(in.nf),nv(in.nv),nen(in.nen),
	pivotpos(in.pivotpos),color(in.color),
	lastdraw(in.lastdraw),nanims(in.nanims),
	lastintesectedface(in.lastintesectedface)
{ 
	int i;
	if(in.localvert)
	{
		localvert=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nv);
		for( i=0;i<nv;i++ )
			localvert[i]=in.localvert[i];
	}
	if(in.localen)
	{
		localen = (flyVector *)aligned_alloc(sizeof(flyVector)*nen);
		for( i=0;i<nen;i++ )
			localen[i]=in.localen[i];
	}
	if(in.localfaces)
	{
		localfaces = new flyFace[nf];
		for( i=0;i<nf;i++ )
		{
			localfaces[i]=in.localfaces[i];
			localfaces[i].vert=&localvert[in.localfaces[i].vert-in.localvert];
			localfaces[i].en=&localen[in.localfaces[i].en-in.localen];
		}
	}
	if(in.faces)
	{
		faces = new flyFace* [nf];
		if (localfaces)
			for( i=0; i!=nf; i++)
				faces[i] = &localfaces[i];
		else
			for( i=0; i!=nf; i++)
				faces[i] = in.faces[i];
	}
}

void flyMesh::operator=(const flyMesh& in)
{
	reset();

	flyBaseObject::operator=(in);
	nf=in.nf;
	nv=in.nv;
	nen=in.nen;
	pivotpos=in.pivotpos;
	color=in.color;
	lastdraw=in.lastdraw;
	nanims=in.nanims;
	lastintesectedface=in.lastintesectedface;

	int i;
	if(in.localvert)
	{
		localvert=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nv);
		for( i=0;i<nv;i++ )
			localvert[i]=in.localvert[i];
	}
	if(in.localen)
	{
		localen = (flyVector *)aligned_alloc(sizeof(flyVector)*nen);
		for( i=0;i<nen;i++ )
			localen[i]=in.localen[i];
	}
	if(in.localfaces)
	{
		localfaces = new flyFace[nf];
		for( i=0;i<nf;i++ )
		{
			localfaces[i]=in.localfaces[i];
			localfaces[i].vert=&localvert[in.localfaces[i].vert-in.localvert];
			localfaces[i].en=&localen[in.localfaces[i].en-in.localen];
		}
	}
	if(in.faces)
	{
		faces = new flyFace* [nf];
		if (localfaces)
			for( i=0; i!=nf; i++)
				faces[i] = &localfaces[i];
		else
			for( i=0; i!=nf; i++)
				faces[i] = in.faces[i];
	}
}

void flyMesh::reset()
{
	delete[] faces;
	faces=0;

	delete[] localfaces;
	localfaces=0;

	aligned_free(localvert);
	localvert=0;

	aligned_free(localen);
	localen=0;

	nv=0;
	nf=0;
	nen=0;
	pivotpos.vec(0,0,0);
	color.vec(1,1,1,1);
	lastdraw=0;
	nanims=0;
	type=FLY_TYPE_MESH;
	lastintesectedface=-1;
}

void flyMesh::compute_normals(int flag)
{
	int i;

	if (flag&FLY_MESH_VERTNORM && localvert)
		for( i=0;i<nv;i++ )
			localvert[i].normal.vec(0,0,0);

	if ((flag&FLY_MESH_FACENORM) || (flag&FLY_MESH_BBOX))
		for( i=0;i<nf;i++ )
			faces[i]->compute_normals(flag);

	if (flag&FLY_MESH_BBOX)
	{

		bbox.reset();
		for( i=0;i<nf;i++ )
		{
			bbox.add_point(faces[i]->bbox.min);
			bbox.add_point(faces[i]->bbox.max);
		}
	}

	if ((flag&FLY_MESH_OCTREE)&&g_flyengine->octree)
		for( i=0;i<nf;i++ )
			if (faces[i]->facetype==FLY_FACETYPE_TRIANGLE_MESH)
			{
				delete faces[i]->octree;
				faces[i]->octree=new flyOcTree();
				faces[i]->octree->build_tree(faces[i]);
			}
}

int flyMesh::ray_intersect(const flyVector& ro,const flyVector& rd,flyVector& ip,float& dist) const 
{
	flyVector tmp_ip;
	float tmp_dist;
	int i,j,f=-1,sf=0;
	dist=FLY_BIG;

	for( i=0;i<nf;i++ )
	{
		j=faces[i]->ray_intersect(ro,rd,tmp_ip,tmp_dist);
		if (j!=-1 && tmp_dist<dist)
			{
			dist=tmp_dist;
			ip=tmp_ip;
			f=i;
			sf=j;
			}
	}

	return f==-1 ? -1 : (f|(sf<<16));
}

int flyMesh::ray_intersect_test(const flyVector& ro,const flyVector& rd,float dist)
{
	int i,j;

	if (lastintesectedface!=-1)
	{
		if (faces[lastintesectedface]->ray_intersect_test(ro,rd,dist))
			return 1;
		j=lastintesectedface;
	}
	else
		j=nf;

	for( i=0;i<j;i++ )
		if (faces[i]->ray_intersect_test(ro,rd,dist))
			{
				lastintesectedface=i;
				return 1;
			}

	for( i=j+1;i<nf;i++ )
		if (faces[i]->ray_intersect_test(ro,rd,dist))
			{
				lastintesectedface=i;
				return 1;
			}

	lastintesectedface=-1;
	return 0;
}

void flyMesh::illum_faces(const flyVector& ip,float d_max,const flyVector& c,int shadows) const
{
	if (lastdraw<g_flyengine->cur_frame_base || g_flyengine->dynlight==0)
		return;

	flyLightMap *lm;
	flyFace *f;
	int i;

	g_flyengine->collisionmode=1;

	for( i=0;i<nf;i++ )
	{
		f=faces[i];
		if (ip.x<f->bbox.min.x-d_max || ip.x>f->bbox.max.x+d_max ||
			ip.y<f->bbox.min.y-d_max || ip.y>f->bbox.max.y+d_max ||
			ip.z<f->bbox.min.z-d_max || ip.z>f->bbox.max.z+d_max)
			continue;
		if(f->lm!=-1)
		{
			lm=g_flyengine->lm[f->lm];
			if (lm->lastupdate!=g_flyengine->cur_step)
			{
			lm->lastupdate=g_flyengine->cur_step;
			lm->illum(ip,c,d_max,shadows);
			g_flyengine->lmchanged.add(f->lm);
			}
		}
		else
		if (f->facetype==FLY_FACETYPE_TRIANGLE_MESH)
		if (f->lastupdate!=g_flyengine->cur_step)
		{
			f->lastupdate=g_flyengine->cur_step;
			g_flyengine->facechanged.add(f->indx);

			int j,k;
			flyVector dir;
			float dist,dot,d_max2=d_max*d_max;
			unsigned char *uc;

			for( j=0;j<f->nvert;j++ )
			{
				k=j+f->vertindx;
				dir=f->vert[k]-ip;
				dist=dir.length2();
				if (dist<d_max2)
				{
					dist=(float)sqrt(dist);
					dir*=1.0f/dist;
					dot=FLY_VECDOT(dir,f->vert[k].normal);
					if ((shadows&1) && dot>0)
						continue;
					if (shadows&8 &&
						g_flyengine->collision_test(ip+dir,f->vert[k]-dir,FLY_TYPE_STATICMESH))
						continue;

					dist=(1.0f-dist/d_max);
					if (shadows&2)
						dist*=dist;
					if (shadows&4)
						dist*=-dot;
					dist*=255.0;

					uc=(unsigned char *)&f->vert[k].color;
					
					k=(int)(c.x*dist)+(int)(*uc);
					*(uc++)=k>255?255:k;

					k=(int)(c.y*dist)+(int)(*uc);
					*(uc++)=k>255?255:k;

					k=(int)(c.z*dist)+(int)(*uc);
					*(uc++)=k>255?255:k;
				}
			}
		}
	}
	g_flyengine->collisionmode=0;
}

int flyMesh::save(const char *file) const 
{
	if (localvert==0 || localfaces==0)
		return 0;
	FILE *fp=fopen(file,"wb");
	if (fp==0)
		return 0;

	int i;
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

	i=1;
	fwrite(&i,1,sizeof(int),fp);	// nanims
	fwrite(&i,1,sizeof(int),fp);	// nkeys
	char str[FLY_MAX_MESHSTRLEN]="static";
	fwrite(str,1,FLY_MAX_MESHSTRLEN,fp);	// animnames

	for( i=0;i<nv;i++ )
		fwrite(&localvert[i].x,3,sizeof(float),fp);

	fclose(fp);
	return 1;
}

int flyMesh::load(const char *file)
{
	int i,j;
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
	fp.seek(nanims*(sizeof(int)+FLY_MAX_MESHSTRLEN));
	nanims=1;

	for( i=0;i<nv;i++ )
		fp.read(&localvert[i].x,3*sizeof(float));
	for( i=0;i<nv;i++ )
	{
		localvert[i].w=0;
		localvert[i].texcoord.z=localvert[i].texcoord.w=0;
		localvert[i].normal.null();
	}
	
	fp.close();

	localen=(flyVector *)aligned_alloc(sizeof(flyVector)*nen);
	j=0;
	for( i=0;i<nf;i++ )
	{
		localfaces[i].sortkey=(localfaces[i].sh<<16);
		localfaces[i].en=&localen[j];
		j+=localfaces[i].ntriface*4;
	}

	compute_normals(FLY_MESH_ALL);

	return 1;
}

void flyMesh::set_numverts(int nverts,int keep)
{
	flyVertex *tmp=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nverts);
	if(keep)
		if (nverts>=nv)
			memcpy(tmp,localvert,sizeof(flyVertex)*nv);
		else 
			memcpy(tmp,localvert,sizeof(flyVertex)*nverts);
	nv=nverts;
	aligned_free(localvert);
	localvert=tmp;
}

void flyMesh::set_numfaces(int nfaces,int local,int keep)
{
	flyFace **tmp1=new flyFace *[nfaces];
	if(keep)
		if (nfaces>=nf)
			memcpy(tmp1,faces,sizeof(flyFace *)*nf);
		else 
			memcpy(tmp1,faces,sizeof(flyFace *)*nfaces);
	delete[] faces;
	faces=tmp1;
	if (local)
	{
		flyFace *tmp2=new flyFace[nfaces];
		if(keep)
			if (nfaces>=nf)
				memcpy(tmp2,localfaces,sizeof(flyFace)*nf);
			else 
				memcpy(tmp2,localfaces,sizeof(flyFace)*nfaces);
		delete[] localfaces;
		localfaces=tmp2;
		for( int i=0;i<nfaces;i++ )
			faces[i]=&localfaces[i];
	}
	nf=nfaces;
}

void flyMesh::set_numen(int numen,int keep)
{
	flyVector *tmp=(flyVector *)aligned_alloc(sizeof(flyVector)*numen);
	if(keep)
		if (nen>=numen)
			memcpy(tmp,localen,sizeof(flyVector)*nen);
		else 
			memcpy(tmp,localen,sizeof(flyVector)*numen);
	nen=numen;
	aligned_free(localen);
	localen=tmp;
}

void flyMesh::draw_plain()
{
	flyVertex *v=localvert==0?g_flyengine->vert:localvert;
	glVertexPointer(3,GL_FLOAT,sizeof(flyVertex),&v->x);
	glTexCoordPointer(2,GL_FLOAT,sizeof(flyVertex),&v->x);
	glNormalPointer(GL_FLOAT,sizeof(flyVertex),&v->normal.x);
	glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(flyVertex),&v->color);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	int i;
	for( i=0;i<nf;i++ )
		switch(faces[i]->facetype)
		{
		case FLY_FACETYPE_LARGE_POLYGON:
			glDrawArrays(GL_TRIANGLE_FAN,faces[i]->vertindx,faces[i]->nvert);
			break;
		case FLY_FACETYPE_BEZIER_PATCH:
			faces[i]->patch->draw(0);
			break;
		case FLY_FACETYPE_TRIANGLE_MESH:
			glDrawElements(GL_TRIANGLES,faces[i]->ntrivert,GL_UNSIGNED_INT,faces[i]->trivert);
			break;
		case FLY_FACETYPE_TRIANGLE_STRIP:
			glDrawElements(GL_TRIANGLE_STRIP,faces[i]->ntrivert,GL_UNSIGNED_INT,faces[i]->trivert);
			break;
		case FLY_FACETYPE_TRIANGLE_FAN:
			glDrawElements(GL_TRIANGLE_FAN,faces[i]->ntrivert,GL_UNSIGNED_INT,faces[i]->trivert);
			break;
	} 
}

void flyMesh::draw() 
{
	lastdraw=g_flyengine->cur_frame;
	g_flyengine->shadercolor=color;
	g_flyengine->draw_faces(nf,faces,localvert,0);
	if ((g_flyengine->debug&2)&&g_flyengine->octree)
	{
		int l=glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		glColor4ub(128,128,128,255);
		for( int i=0;i<nf;i++ )
			if (faces[i]->octree)
				faces[i]->octree->draw();
		if (l) glEnable(GL_LIGHTING);
	}
}

void flyMesh::draw_shadow_volume(const flyVector& lightdir) 
{
	static flyVector v[8],n[3],v1,v2;

	flyVector shadowdir=-g_flyengine->shadowdist*lightdir;

	int i,j,k,l=0;
	flyFace *f=localfaces;
	for( i=0;i<nf;i++,f++ )
	if( f->sh==-1 || (g_flyengine->shaders[f->sh]->flags&FLY_SHADER_TRANSPARENT)==0)
	{
		k=0;
		for( j=0;j<f->ntriface;j++,l+=4 )
			if (FLY_VECDOT(localen[l],lightdir)<=0)
				k+=3;
			else
			{
				v[0]=localvert[f->trivert[k]];
				k++;
				v[1]=v[0]+shadowdir;
				v[2]=localvert[f->trivert[k]];
				k++;
				v[3]=v[2]+shadowdir;
				v[4]=localvert[f->trivert[k]];
				k++;
				v[5]=v[4]+shadowdir;
				v[6]=v[0];
				v[7]=v[1];

				glVertexPointer(3,GL_FLOAT,sizeof(flyVector),v);

				glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
				glCullFace(GL_BACK);
				glDrawArrays(GL_QUAD_STRIP,0,8);

				glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
				glCullFace(GL_FRONT);
				glDrawArrays(GL_QUAD_STRIP,0,8);
			}
	}
}
