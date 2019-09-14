#include "../Fly3D.h"
#include "resource.h"

int flyEngine::load_ini()
{
	flyString ret,str;

	str=flysdkpath;
	str+="Fly3D.ini";
	flyFile file;
	if (file.open(str)==0)
		return 0;
	
	flyParamDesc pd;
	int i,j;

	j=get_global_param_desc1(0,0);
	for( i=0;i<j;i++ )
	{
		get_global_param_desc1(i,&pd);
		if (file.get_profile_string("fly3d",pd.name,ret))
			pd.set_string(ret);
	}

	return 1;
}

int flyEngine::save_ini()
{
	flyString ret,str;

	str=flysdkpath;
	str+="Fly3D.ini";

	FILE *fp;
	fp=fopen(str,"wt");
	if (fp==0)
		return 0;
	
	fprintf(fp,"[fly3d]\n");

	flyParamDesc pd;
	int i,j;

	j=get_global_param_desc1(0,0);
	for( i=0;i<j;i++ )
	{
		get_global_param_desc1(i,&pd);
		if (pd.type!=' ')
			fprintf(fp,"%s=%s\n",(const char *)pd.name,pd.get_string());
	}
	fclose(fp);

	return 1;
}

flyString load_res_string(int id)
{
	char str[255];
	LoadString(GetModuleHandle("flyEngine.dll"),id,str,255);
	return flyString(str);
}

int flyEngine::load_data()
{
	flyString ret,str;
	int i,j;
	flyParamDesc pd;
	
	objid=1;

	console_printf("Loading %s...",(const char *)flyfile);
	loading=1;

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_OPENSCENEFILE),5);

	flyFile fly_file;
	if (!fly_file.open(flyfilename))
	{
		glDrawBuffer(GL_BACK);
		return 0;
	}
	
	fly_file.get_profile_string("fly3d","bspfile",bspfile);

	j=get_global_param_desc2(0,0);
	for( i=0;i<j;i++ )
	{
		get_global_param_desc2(i,&pd);
		if (fly_file.get_profile_string("fly3d",pd.name,ret))
			pd.set_string(ret);
	}

	if (bspfile[0]!=0)
	{
		load_level();
		if (bsp==0)
		{
			fly_file.close();
			reset();
			glDrawBuffer(GL_BACK);
			return 0;
		}
	}

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_LOADOBJECTS),70);

	fly_file.get_profile_string("fly3d","bsppicflipy",ret);
	sscanf((const char *)ret,"%i",&bsppicflipy);

	i=0;
	while(1)
	{
		str.format("dll%i",i++);
		fly_file.get_profile_string("classes",str,ret);
		if (ret[0]==0) break;
		j=dll.add_dll(ret);
		if (j!=-1)
			if(dll.dll[j]->get_version && FLY_VERSION_NUM!=dll.dll[j]->get_version())
			{
				dll.delete_dll(ret);
				console_printf("%s: Incompatible version",(char *)(const char *)ret);
			}
			else
				console_printf("%s: Loaded (%i classes)",(char *)(const char *)ret,dll.dll[j]->num_classes());
	}

	dll.load_all_classes(&fly_file);
	flyBspObject *o;
	o=(flyBspObject *)stock_objs.get_next_object(0);
	while(o)
		{
		o->load_params(&fly_file,o->name);
		o=(flyBspObject *)stock_objs.get_next_object(o);
		}
	o=(flyBspObject *)active_objs.get_next_object(0);
	while(o)
		{
		o->load_params(&fly_file,o->name);
		o=(flyBspObject *)active_objs.get_next_object(o);
		}

	fly_file.get_profile_string("fly3d","camera",ret);
	cam=(flyBspObject *)active_objs.get_object_name(ret);
	fly_file.get_profile_string("fly3d","player",ret);
	player=(flyBspObject *)active_objs.get_object_name(ret);

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_INITSCENE),80);
	
	dll.send_message(FLY_MESSAGE_INITSCENE,0,0);

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_INITOBJECTS),90);

	o=(flyBspObject *)stock_objs.get_next_object(0);
	while(o)
	{
		o->id=objid++;
		o->init();
		o=(flyBspObject *)stock_objs.get_next_object(o);
	}
	o=(flyBspObject *)active_objs.get_next_object(0);
	while(o)
	{
		cur_step++;
		if (o->source)
			o->id=o->source->id;
		else
			o->id=0;
		add_to_bsp(o);
		o=(flyBspObject *)active_objs.get_next_object(o);
	}
	o=0;
	while(o=(flyBspObject *)active_objs.get_next_object(o))
	{
		o->init();
		add_to_bsp(o);
	}

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_READY),100);

	console_printf("Ready.");
	loading=0;

	return 1;
}

void flyEngine::load_level()
{
	flyString str;

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_LOADSHADERS),10);

	str=flyfilepath+bspfile+".shr";
	load_shaders(str);
	
	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_LOADFMP),20);

	str=flyfilepath+bspfile+".fmp";
	load_fmp(str);

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_LOADBSP),30);

	str=flyfilepath+bspfile+".bsp";
	load_bsp(str);

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_LOADPVS),35);

	str=flyfilepath+bspfile+".pvs";
	load_pvs(str);

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_LOADFPT),40);

	str=flyfilepath+bspfile+".fpt";
	load_fpt(str);

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_LOADLMP),50);

	str=flyfilepath+bspfile+".lmp";
	load_lightmaps(str);

	if(g_flyrender)
		g_flyrender->draw_progress((const char *)load_res_string(IDS_PROCESSDATA),60);

	compute_normals();
	compute_node_light();
}

int flyEngine::load_fmp(const char *file)
{
	int i,j;
	flyFile fp;

	if (!fp.open(file))
		return 0;

	fp.read(&i,sizeof(int));
	fp.read(&j,sizeof(int));
	if (i!=FLY_FILEID_FMP && j!=2)
		{
		fp.close();
		return 0;
		}

	fp.read(&nvert,sizeof(int));
	vert=(flyVertex *)aligned_alloc(sizeof(flyVertex)*nvert);

	for( i=0;i<nvert;i++ )
	{
		fp.read(&vert[i].x,3*sizeof(float));
		fp.read(&vert[i].texcoord.x,2*sizeof(float));
		fp.read(&vert[i].color,sizeof(int));
	}

	fp.read(&nfaces,sizeof(int));
	faces=new flyFace[nfaces];
	for( i=0;i<nfaces;i++ )
	{
		fp.read(&faces[i].facetype,sizeof(int));
		fp.read(&faces[i].flag,sizeof(int));
		fp.read(&faces[i].sh,sizeof(int));
		fp.read(&faces[i].vertindx,sizeof(int));
		fp.read(&faces[i].nvert,sizeof(int));
		faces[i].vert=&vert[faces[i].vertindx];

		if (faces[i].facetype==FLY_FACETYPE_BEZIER_PATCH)
		{
			fp.read(&faces[i].patch_npu,sizeof(int));
			fp.read(&faces[i].patch_npv,sizeof(int));
		}
		else
		if (faces[i].facetype==FLY_FACETYPE_TRIANGLE_MESH)
		{
			fp.read(&faces[i].ntriface,sizeof(int));
			faces[i].ntrivert=faces[i].ntriface*3;
			faces[i].trivert=new int[faces[i].ntrivert];
			fp.read(faces[i].trivert,faces[i].ntrivert*sizeof(int));
		}

		faces[i].lm=-1;
		faces[i].lastbsprecurse=0;
		faces[i].indx=i;
	}

	fp.close();

	return 1;
}
	
int flyEngine::save_fmp(const char *file) const
{
	FILE *fp;
	int i;
	
	fp=fopen(file,"wb");
	if (!fp)
		return 0;

	i=FLY_FILEID_FMP;
	fwrite(&i,1,sizeof(int),fp);
	i=2;
	fwrite(&i,1,sizeof(int),fp);

	fwrite(&nvert,1,sizeof(int),fp);

	for( i=0;i<nvert;i++ )
	{
		fwrite(&vert[i].x,1,3*sizeof(float),fp);
		fwrite(&vert[i].texcoord.x,1,2*sizeof(float),fp);
		fwrite(&vert[i].color,1,sizeof(int),fp);
	}

	fwrite(&nfaces,1,sizeof(int),fp);
	for( i=0;i<nfaces;i++ )
		{
		fwrite(&faces[i].facetype,1,sizeof(int),fp);
		fwrite(&faces[i].flag,1,sizeof(int),fp);
		fwrite(&faces[i].sh,1,sizeof(int),fp);
		fwrite(&faces[i].vertindx,1,sizeof(int),fp);
		fwrite(&faces[i].nvert,1,sizeof(int),fp);
		if (faces[i].facetype==FLY_FACETYPE_BEZIER_PATCH)
			{
			fwrite(&faces[i].patch_npu,1,sizeof(int),fp);
			fwrite(&faces[i].patch_npv,1,sizeof(int),fp);
			}
		else
		if (faces[i].facetype==FLY_FACETYPE_TRIANGLE_MESH)
			{	
			fwrite(&faces[i].ntriface,1,sizeof(int),fp);
			fwrite(faces[i].trivert,faces[i].ntrivert,sizeof(int),fp);
			}
		}

	fclose(fp);

	return 1;
}

int flyEngine::load_bsp(const char *file) 
{
	int i,j;
	flyFile fp;

	if (!fp.open(file))
		return 0;

	fp.read(&i,sizeof(int));
	fp.read(&j,sizeof(int));
	if (i!=FLY_FILEID_BSP && j!=2)
		{
		fp.close();
		return 0;
		}

	load_bsp(&bsp,&fp);
	
	fp.close();

	return 1;
}

void flyEngine::load_bsp(flyBspNode **n,flyFile *fp)
{
	char c;
	int f,i,j;

	fp->read(&c,1);
	if (c==0)
		return;

	*n=new flyBspNode;

	if (c==1)
	{
		fp->read(&(*n)->normal,3*sizeof(float));
		fp->read(&(*n)->d0,sizeof(float));

		load_bsp(&(*n)->child[0],fp);
		load_bsp(&(*n)->child[1],fp);
	}
	else if (c==2)
	{
		fp->read(&(*n)->leaf,sizeof(int));
		fp->read(&f,sizeof(int));
		if (f)
			{
			flyStaticMesh *sm=new flyStaticMesh;
			sm->active=-1;
			sm->collide=2;
			sm->objmesh->pivotpos.null();
			sm->pos.null();
			sm->objmesh->nf=f;
			sm->objmesh->faces=new flyFace *[f];
			for( i=0;i<f;i++ )
				{
				fp->read(&j,sizeof(int));
				sm->objmesh->faces[i]=&faces[j];
				}
			(*n)->elem.add(sm);
			}
	}
}

void flyEngine::save_bsp(const flyBspNode *n,FILE *fp) const 
{
	char c;
	int i;

	if (n==0)
		{
		c=0;
		fwrite(&c,1,1,fp);
		return;
		}

	if (n->leaf==-1)
	{
		c=1;
		fwrite(&c,1,1,fp);

		fwrite(&n->normal, 3, sizeof(float), fp);
		fwrite(&n->d0, 1, sizeof(float), fp);

		save_bsp(n->child[0],fp);
		save_bsp(n->child[1],fp);
	}
	else
	{
		c=2;
		fwrite(&c,1,1,fp);

		fwrite(&n->leaf,1,sizeof(int),fp);

		if (n->elem.num && n->elem[0]->type==FLY_TYPE_STATICMESH)
			{
			flyStaticMesh *sm=(flyStaticMesh *)n->elem[0];
			fwrite(&sm->objmesh->nf,1,sizeof(int),fp);
			for( i=0;i<sm->objmesh->nf;i++ )
				fwrite(&sm->objmesh->faces[i]->indx,1,sizeof(int),fp);
			}
		else 
			{
			i=0;
			fwrite(&i,1,sizeof(int),fp);
			}
	}
}

int flyEngine::save_bsp(const char *file) const 
{
	FILE *fp;
	int i;
	
	fp=fopen(file,"wb");
	if (!fp)
		return 0;

	i=FLY_FILEID_BSP;
	fwrite(&i,1,sizeof(int),fp);
	i=2;
	fwrite(&i,1,sizeof(int),fp);

	save_bsp(bsp,fp);

	fclose(fp);

	return 1;
}

int flyEngine::load_lightmaps(const char *file)
{
	if (nfaces==0)
		return 0;
	int i,j,x,y,sx,sy,f,p,ver;
	flyFile fp;
	
	if (!fp.open(file)) 
		return 0;

	fp.read(&i,sizeof(int));
	fp.read(&ver,sizeof(int));
	if (i!=FLY_FILEID_LMP && ver!=1 && ver!=2)
	{	fp.close(); return 0; }

	fp.read(&i,sizeof(int));
	fp.read(&j,sizeof(int));
	if (i!=nfaces || j!=nvert) 
	{ fp.close(); return 0;};
	
	for( i=0;i<nfaces;i++ )
		fp.read(&faces[i].lm,sizeof(int));

	for( i=0;i<nvert;i++ )
		fp.read(&vert[i].texcoord.z,sizeof(float)*2);

	int nlm;
	flyLightMap *l;
	int nlmpic;
	flyLightMapPic *lp;
	if (ver==1)
	{
		fp.read(&nlmpic,sizeof(int));
		for( i=0;i<nlmpic;i++ )
		{
			fp.read(&sx,sizeof(int));
			fp.read(&sy,sizeof(int));
			lp=new flyLightMapPic(sx,sy);
			fp.read(lp->bmp,lp->bytesxy);
			lmpic.add(lp);
		}

		fp.read(&nlm,sizeof(int));
		for( i=0;i<nlm;i++ )
		{
			fp.read(&f,sizeof(int));
			fp.read(&p,sizeof(int));
			fp.read(&x,sizeof(int));
			fp.read(&y,sizeof(int));
			fp.read(&sx,sizeof(int));
			fp.read(&sy,sizeof(int));
			l=new flyLightMap(f,p,x,y,sx,sy,3);
			lm.add(l);
		}
	}
	else if (ver==2)
	{
		fp.read(&nlm,sizeof(int));
		for( i=0;i<nlm;i++ )
		{
			fp.read(&f,sizeof(int));
			fp.read(&p,sizeof(int));
			fp.read(&x,sizeof(int));
			fp.read(&y,sizeof(int));
			fp.read(&sx,sizeof(int));
			fp.read(&sy,sizeof(int));
			l=new flyLightMap(f,p,x,y,sx,sy,3);
			lm.add(l);
		}

		fp.read(&nlmpic,sizeof(int));
		if (nlmpic)
		{
			fp.read(&sx,sizeof(int));
			fp.read(&sy,sizeof(int));
			flyPicture p;
			p.LoadJPG(&fp.buf[fp.pos],fp.len-fp.pos);
			for( i=0;i<nlmpic;i++ )
			{
				lp=new flyLightMapPic(sx,sy);
				memcpy(lp->bmp,&p.buf[i*sx*sy*3],lp->bytesxy);
				lmpic.add(lp);
			}
		}
	}

	fp.close();

	unsigned char *uc;
	x=(int)(flyRender::s_brightness*255.0f);
	y=(int)(flyRender::s_ambient*255.0f);
	for( i=0;i<nlmpic;i++ )
	{
		uc=lmpic[i]->bmp;
		for( j=0;j<lmpic[i]->bytesxy;j++ )
		{
			p=(int)(*uc)*x/255+y;
			*(uc++)=p<0?0:p>255?255:p;
		}
	}
	for( i=0;i<nvert;i++ )
	{
		uc=(unsigned char *)&vert[i].color;
		p=(int)(*uc)*x/255+y;
		*(uc++)=p<0?0:p>255?255:p;
		p=(int)(*uc)*x/255+y;
		*(uc++)=p<0?0:p>255?255:p;
		p=(int)(*uc)*x/255+y;
		*(uc++)=p<0?0:p>255?255:p;
		vert[i].color_static=vert[i].color;
	}

	for( i=0;i<nlm;i++ )
		lm[i]->load(lmpic[lm[i]->pic]);

	if (fogmap)
	{
	for( i=0;i<nlmpic;i++ )
		{
		lp=new flyLightMapPic(lmpic[i]->sizex,lmpic[i]->sizey,4);
		memset(lp->bmp,0,lp->bytesxy);
		fmpic.add(lp);
		}
	for( i=0;i<nlm;i++ )
		{
		l=new flyLightMap(lm[i]->facenum,lm[i]->pic,lm[i]->offsetx,lm[i]->offsety,lm[i]->sizex,lm[i]->sizey,4);
		memset(l->bmp,0,l->bytesxy);
		fm.add(l);
		}
	}

	lmbase=-1;
	fmbase=-1;
	char str[64];
	if (g_flytexcache)
	{
		lmbase=g_flytexcache->ntex;
		for( i=0;i<nlmpic;i++ )
		{
			sprintf(str,"~lightmappic%02i",i);
			g_flytexcache->add_tex(str,lmpic[i]->sizex,lmpic[i]->sizey,3,lmpic[i]->bmp,1+4*flyTexCache::s_textruecolor);
		}
		if (fogmap)
		{
			fmbase=g_flytexcache->ntex;
			for( i=0;i<nlmpic;i++ )
				{
				sprintf(str,"~fogmappic%02i",i);
				g_flytexcache->add_tex(str,fmpic[i]->sizex,fmpic[i]->sizey,4,fmpic[i]->bmp,1+4*flyTexCache::s_textruecolor);
				}
		}
		else 
			fmbase=-1;
	}

	return 1;
}

int flyEngine::save_lightmaps(const char *file,int quality) const 
{
	if (nfaces==0)
		return 0;
	FILE *fp;
	int i;
	
	fp=fopen(file,"wb");
	if (fp==0) return 0;

	i=FLY_FILEID_LMP;
	fwrite(&i,1,sizeof(int),fp);
	i=((quality==-1)?1:2);
	fwrite(&i,1,sizeof(int),fp);

	fwrite(&nfaces,1,sizeof(int),fp);
	fwrite(&nvert,1,sizeof(int),fp);
	
	for (i=0;i<nfaces;i++ )
		fwrite(&faces[i].lm,sizeof(int),1,fp);

	for (i=0;i<nvert;i++ )
		fwrite(&vert[i].texcoord.z,sizeof(float),2,fp);

	if (quality==-1)
	{
		fwrite(&lmpic.num,1,sizeof(int),fp);
		for( i=0;i<lmpic.num;i++ )
		{
			fwrite(&lmpic[i]->sizex,sizeof(int),1,fp);
			fwrite(&lmpic[i]->sizey,sizeof(int),1,fp);
			fwrite(lmpic[i]->bmp,lmpic[i]->bytesxy,1,fp);
		}

		fwrite(&lm.num,1,sizeof(int),fp);
		for( i=0;i<lm.num;i++ )
		{
			fwrite(&lm[i]->facenum,sizeof(int),1,fp);
			fwrite(&lm[i]->pic,sizeof(int),1,fp);
			fwrite(&lm[i]->offsetx,sizeof(int),2,fp);
			fwrite(&lm[i]->sizex,sizeof(int),2,fp);
		}
	}
	else
	{	
		fwrite(&lm.num,1,sizeof(int),fp);
		for( i=0;i<lm.num;i++ )
		{
			fwrite(&lm[i]->facenum,sizeof(int),1,fp);
			fwrite(&lm[i]->pic,sizeof(int),1,fp);
			fwrite(&lm[i]->offsetx,sizeof(int),2,fp);
			fwrite(&lm[i]->sizex,sizeof(int),2,fp);
		}

		fwrite(&lmpic.num,1,sizeof(int),fp);
		if (lmpic.num)
		{
			int sx=lmpic[0]->sizex;
			int sy=lmpic[0]->sizey;
			fwrite(&sx,sizeof(int),1,fp);
			fwrite(&sy,sizeof(int),1,fp);
			flyPicture p;
			p.CreatePicture24(sx,sy*lmpic.num);
			for( i=0;i<lmpic.num;i++ )
				memcpy(&p.buf[i*3*sx*sy],lmpic[i]->bmp,3*sx*sy);
			p.SaveJPG(fp,quality);
		}
	}

	fclose(fp);
	return 1;
}

int flyEngine::load_pvs(const char *file)
{
	int i,j;
	flyFile fp;

	alloc_pvs(255);
	if (pvssize==0) return 0;

	if (!fp.open(file)) return 0;

	fp.read(&i,sizeof(int));
	fp.read(&j,sizeof(int));
	if (i!=FLY_FILEID_LMP && j!=1)
		{
		fp.close();
		return 0;
		}

	fp.read(&i,sizeof(int));
	if (i==nleaf)
		fp.read(pvs,pvssize);
	
	fp.close();

	for( i=0;i<nleaf;i++ )
		if (leaf[i]->leaf!=i)
			break;

	return 1;
}

int flyEngine::save_pvs(const char *file) const 
{
	if (pvssize==0) return 0;

	FILE *fp=fopen(file,"wb");
	if (fp==0) return 0;

	int i=FLY_FILEID_PVS;
	fwrite(&i,1,sizeof(int),fp);
	i=1;
	fwrite(&i,1,sizeof(int),fp);

	fwrite(&nleaf,1,sizeof(int),fp);
	fwrite(pvs,pvssize,1,fp);
	fclose(fp);
	return 1;
}

void flyEngine::find_leaf(flyBspNode *n)
{
	if (n->child[0]==0 && n->child[1]==0)
		{
		if (n->leaf!=-1)
			if (leaf)
				leaf[n->leaf]=n;
			else nleaf++;
		return;
		}
	n->leaf=-1;
	if (n->child[0])
		find_leaf(n->child[0]);
	if (n->child[1])
		find_leaf(n->child[1]);
}

void flyEngine::alloc_pvs(int value)
{
	delete[] pvs;
	pvs=0;
	pvssize=0;
	pvsrowsize=0;
	delete[] leaf;
	leaf=0;
	nleaf=0;

	if (bsp)
	{
		find_leaf(bsp);
		if (nleaf==0)
			return;
		leaf=new flyBspNode *[nleaf];
		memset(leaf,0,nleaf*sizeof(flyBspNode *));
		find_leaf(bsp);
	}

	if (nleaf)
		{
		pvsrowsize=nleaf/8+1;
		pvssize=pvsrowsize*nleaf;
		pvs=new char[pvssize];
		memset(pvs,value,pvssize);
		}
}

int flyEngine::load_shaders(const char *file)
{
	flyFile fp;
	flyString section,longname;

	if (!fp.open(file))
		return 0;

	int i=0;
	do 
	{
		section.format("shader%i",i);
		fp.get_profile_string(section,"longname",longname);
		if (longname[0]==0)
			break;
		flyShader *s=new flyShader;
		s->name=section;
		s->long_name=longname;
		s->load(&fp,section);
		shaders.add(s);
		i++;
	} while(1);

	fp.close();
	
	return i;
}

void flyEngine::compute_node_light()
{
	int i,j,k,xy;
	unsigned char *uc;
	flyLightMap *l;
	flyMesh *m;

	if (lm.num==0)
	{
	for( i=0;i<nleaf;i++ )
		leaf[i]->color.vec(1,1,1,1);
	return;
	}

	for( i=0;i<nleaf;i++ )
		{
		leaf[i]->color.null();
		leaf[i]->leaf=0;
		}

	for( i=0;i<nleaf;i++ )
	if (leaf[i]->elem.num && leaf[i]->elem[0]->type==FLY_TYPE_STATICMESH)
	{
		m=((flyStaticMesh *)leaf[i]->elem[0])->objmesh;
		for( j=0;j<m->nf;j++ )
			if (m->faces[j]->lm!=-1)
			{
			l=lm[m->faces[j]->lm];
			uc=l->bmp;
			xy=l->sizex*l->sizey;
			for( k=0;k<xy;k++ )
				{
				leaf[i]->color.x+=*(uc++);
				leaf[i]->color.y+=*(uc++);
				leaf[i]->color.z+=*(uc++);
				leaf[i]->leaf++;
				}
			}
	}

	for( i=0;i<nleaf;i++ )
		{
		if (leaf[i]->leaf)
		{
			leaf[i]->color=leaf[i]->color/(leaf[i]->leaf*255.0f);
			if (leaf[i]->color.x>1.0f)
				leaf[i]->color.x=1.0f;
			if (leaf[i]->color.y>1.0f)
				leaf[i]->color.y=1.0f;
			if (leaf[i]->color.z>1.0f)
				leaf[i]->color.z=1.0f;
		}

		leaf[i]->color.w=1.0f;
		leaf[i]->leaf=i;
		}
}

void flyEngine::compute_normals()
{
	int i,j,k,n,u,v,u0,v0;
	flyVector vec;

	for( i=0;i<nvert;i++ )
		vert[i].normal.null();

	for( i=0;i<nfaces;i++ )
	{
		switch(faces[i].facetype)
		{
		case FLY_FACETYPE_LARGE_POLYGON:
			nen+=faces[i].nvert;
			break;
		case FLY_FACETYPE_TRIANGLE_MESH:
			nen+=faces[i].ntriface*4;
			break;
		case FLY_FACETYPE_BEZIER_PATCH:
			faces[i].patch=new flyBezierPatch;
			faces[i].patch->build_cp(faces[i].patch_npu,faces[i].patch_npv,faces[i].vert);
			faces[i].patch->build_surface();
			faces[i].ntriface=2*(faces[i].patch->nvertu-1)*(faces[i].patch->nvertv-1);
			faces[i].ntrivert=faces[i].ntriface*3;
			faces[i].trivert=new int[faces[i].ntrivert];
			nen+=faces[i].ntriface*4;
			break;
		}
	}

	en = (flyVector *)aligned_alloc(sizeof(flyVector)*nen);

	n=0;
	for( i=0;i<nfaces;i++ )
	{
		faces[i].sortkey=(faces[i].sh<<16)|(faces[i].lm==-1?0xffff:lm[faces[i].lm]->pic);
		faces[i].en=&en[n];
		switch(faces[i].facetype)
		{
		case FLY_FACETYPE_LARGE_POLYGON:
			n+=faces[i].nvert;
			break;

		case FLY_FACETYPE_TRIANGLE_MESH:
			n+=faces[i].ntriface*4;
			faces[i].vert=vert;
			break;

		case FLY_FACETYPE_BEZIER_PATCH:
			n+=faces[i].ntriface*4;
			
			u0=faces[i].patch->nvertu;
			v0=faces[i].patch->nvertv;
			k=0;
			for( v=0;v<v0-1;v++ )
				for( u=0;u<u0-1;u++ )
				{
					faces[i].trivert[k++]=u+v*u0;
					faces[i].trivert[k++]=u+1+(v+1)*u0;
					faces[i].trivert[k++]=u+(v+1)*u0;

					faces[i].trivert[k++]=u+v*u0;
					faces[i].trivert[k++]=u+1+v*u0;
					faces[i].trivert[k++]=u+1+(v+1)*u0;
				}
			faces[i].vert=faces[i].patch->surf;
			faces[i].pivot.vec(0);
			for( k=0;k<u0*v0;k++ )
				faces[i].pivot+=faces[i].vert[k];
			faces[i].pivot*=1.0f/k;
			break;
		}
	}

	for( i=0;i<nfaces;i++ )
	{
		faces[i].compute_normals(FLY_MESH_FACENORM|FLY_MESH_VERTNORM|FLY_MESH_BBOX);
		if (faces[i].facetype!=FLY_FACETYPE_LARGE_POLYGON && octree)
		{
			faces[i].octree=new flyOcTree();
			faces[i].octree->build_tree(&faces[i]);
		}
	}

	if (debug)
	{
		for( i=0;i<nfaces;i++ )
		if (faces[i].facetype==FLY_FACETYPE_LARGE_POLYGON)
		{
		for( j=0;j<faces[i].nvert;j++ )
			if ((float)fabs(faces[i].distance(faces[i].vert[j]))>0.01f)
				break;
		if (j<faces[i].nvert)
			console_printf("warning: face %i has non-planar vertices!",i);
		}
	}

	bbox.reset();
	flyMesh *m;
	for( i=0;i<nleaf;i++ )
	if (leaf[i]->elem.num && leaf[i]->elem[0]->type==FLY_TYPE_STATICMESH)
	{
		m=((flyStaticMesh *)leaf[i]->elem[0])->objmesh;
		if (m)
		{
			m->bbox.reset();
			for( j=0;j<m->nf;j++ )
			{
				m->bbox.add_point(m->faces[j]->bbox.min);
				m->bbox.add_point(m->faces[j]->bbox.max);
			}

			leaf[i]->elem[0]->bbox=m->bbox;

			bbox.add_point(m->bbox.min);
			bbox.add_point(m->bbox.max);
		}
	}

	bboxdiag=(bbox.max-bbox.min).length();
	bboxcenter=(bbox.min+bbox.max)*0.5f;

	for( i=0;i<lm.num;i++ )
		if (lm[i]->facenum!=-1)
			if (faces[lm[i]->facenum].facetype==FLY_FACETYPE_LARGE_POLYGON)
			{
			lm[i]->set_base(&faces[lm[i]->facenum],lmpic[lm[i]->pic]);
			if (fogmap)
				fm[i]->set_base(&faces[fm[i]->facenum],fmpic[fm[i]->pic]);
			}
}

int flyEngine::load_fpt(const char *file)
{
	flyFile fp;
	
	if(!fp.open(file))
		return 0;

	int i,j,k;
	int nneighbors,neighbor,nverts;
	flyVertex v;

	if(fp.get_int()!=nleaf)
		return 0;

	for(i=0;i<nleaf;i++)
	{
		leaf[i]->neighbors.clear();
		leaf[i]->portals.clear();

		leaf[i]->centre.x=fp.get_float();
		leaf[i]->centre.y=fp.get_float();
		leaf[i]->centre.z=fp.get_float();

		nneighbors=fp.get_int();

		for(j=0;j<nneighbors;j++)
		{
			flyPolygon p;

			neighbor=fp.get_int();
			nverts=fp.get_int();
			leaf[i]->neighbors.add(leaf[neighbor]);

			for(k=0;k<nverts;k++)
			{
				v[0]=fp.get_float();
				v[1]=fp.get_float();
				v[2]=fp.get_float();
				p.verts.add(v);
			}

			p.build_normal();
			p.build_edgeplanes();

			leaf[i]->portals.add(p);
		}
	}

	fp.close();
	return 1;
}

int flyEngine::save_fpt(const char *file) const
{
	if(file[0]==0)
		return 0;

	FILE *fp=fopen(file,"wt");
	if(fp)
	{
		fprintf(fp,"%i",nleaf);
		for(int i=0;i<nleaf;i++)
		{
			fprintf(fp,"\n\n%f %f %f",leaf[i]->centre.x,leaf[i]->centre.y,leaf[i]->centre.z);
			fprintf(fp,"\n%i",leaf[i]->neighbors.num);
			for(int j=0;j<leaf[i]->neighbors.num;j++)
			{
				fprintf(fp,"\n%i %i",leaf[i]->neighbors[j]->leaf,leaf[i]->portals[j].verts.num);
				for(int k=0;k<leaf[i]->portals[j].verts.num;k++)
					fprintf(fp," %.6f %.6f %.6f",
							leaf[i]->portals[j].verts[k].x,
							leaf[i]->portals[j].verts[k].y,
							leaf[i]->portals[j].verts[k].z);
			}
		}

		fclose(fp);
		return 1;
	}

	return 0;
}
