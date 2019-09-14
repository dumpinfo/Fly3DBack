#include "../Fly3D.h"

flyEngine::~flyEngine()
{
	close_fly_file(); 
	if (demo_file_in)
		delete demo_file_in;
	if (demo_file_out)
		fclose(demo_file_out);
	delete flyFile::ziplist;
	flyFile::ziplist=0;
}

flyEngine::flyEngine()
{
	hwnd=0; 
	hinst=0;

	nnodedraw=ntotfacedraw=ntotelemdraw=0;
	timedemo=0;
	timedemoframes=0;
	demo_file_in=0;
	demo_file_out=0;
	
	loading=0;

	vert=0; 
	nvert=0;

	faces=0;
	nfaces=0;

	en=0;
	nen=0;

	bsp=0;

	lmbase=fmbase=-1;

	pvs=0;
	leaf=0;
	nleaf=pvssize=pvsrowsize=0;

	debug=0;
	sse=1;
	vertprog=1;
	curveerr=1;
	geomdetail=25;
	mpdelay=40;
	playername="unnamed";
	antialias=0;
	multitexture=1;
	mute=0;
	nodeonly=0;
	pvsoff=0;
	status=1;
	flyRender::s_stencil=0;
	flyTexCache::s_texfilter=1;
	flyTexCache::s_texmipmap=1;
	flyRender::s_wireframe=0;
	shadermask=-1;
	noinput=0;
	picrender=0;
	octree=1;

	frame_count = 0;

	gravity = 0;
	shadowdist = 1000;

	start_time=timeGetTime();
	cur_time_float=0;
	cur_time=0;
	cur_frame_base = 0;
	cur_frame = 0;
	
	srand(start_time);

	reset();

	appid=FLY_APPID_NONE;

	static char buf[512];
	buf[0]=0;
	fly_get_profile("Settings","flysdkpath",(unsigned char *)buf,255);
	flysdkpath=buf;

	if (flysdkpath[0]==0)
	{
		int p;
		flysdkpath=GetCommandLine();
		if (flysdkpath[0]=='\"')
		{
			flysdkpath=flysdkpath.right(flysdkpath.length()-1);
			p=flysdkpath.find('\"'); 
			if (p!=-1)
				flysdkpath.set_char(p,0);
		}
		if ((p=flysdkpath.find_reverse('\\'))!=-1)
			flysdkpath.set_char(p+1,0);
		else 
		{
			GetCurrentDirectory(255,buf);
			flysdkpath=buf;
			flysdkpath+="\\";
		}
		flysdkpath.lower();
	}
	
	buf[0]=0;
	fly_get_profile("Settings","flysdkdatapath",(unsigned char *)buf,255);
	flysdkdatapath=buf;
	
	buf[0]=0;
	fly_get_profile("Settings","flysdkpluginpath",(unsigned char *)buf,255);
	flysdkpluginpath=buf;

	if(flysdkdatapath[0]==0)
	{
		flysdkdatapath=flysdkpath;
		flysdkdatapath+="data\\";
	}
	if(flysdkpluginpath[0]==0)
	{
		flysdkpluginpath=flysdkpath;
		flysdkpluginpath+="plugin\\";
	}
}

void flyEngine::reset()
{
	objid=0;
	
	active_objs.reset();
	stock_objs.reset();
	resource_objs.reset();	

	delete bsp;
	bsp=0;
	aligned_free(vert);
	vert=0;
	nvert=0;
	delete[] faces;
	faces=0;
	nfaces=0;
	aligned_free(en);
	en=0;
	nen=0;

	int i;
	for( i=0;i<shaders.num;i++ )
		delete shaders[i];
	shaders.free();

	delete[] pvs;
	pvs=0;
	pvssize=0;
	pvsrowsize=0;

	selnodes.free();
	selobjs.free();

	delete[] leaf;
	leaf=0;
	nleaf=0;

	facedraw.free();
	facedrawtransp.free();

	for( i=0;i<lm.num;i++ )
		delete lm[i];
	lm.free();
	for( i=0;i<fm.num;i++ )
		delete fm[i];
	fm.free();
	for( i=0;i<lmpic.num;i++ )
		delete lmpic[i];
	lmpic.free();
	for( i=0;i<fmpic.num;i++ )
		delete fmpic[i];
	fmpic.free();

	lmchanged.free();
	fmchanged.free();
	lmbase=-1;
	fmbase=-1;

	facechanged.free();

	dll.reset();

	flyfilepath="";
	flyfile="";
	flyfilename="";
	console_command="";
	status_msg="";
	status_msg_time=0;

	cur_frame=cur_frame_base=0;
	cur_step=cur_step_base=0;
	cur_bsprecurse=cur_dt=0;

	crosshairpic=-1;
	crosshairsize=8;
	fraglimit=timelimit=restarttime=0;
	frame_rate=0;

	bboxdiag=0;
	hitobj=0;
	hitmesh=0;
	hitface=-1;
	hitshader=-1;
	hitsubface=-1;
	hitdist=0;

	dynlight=1;
	shadows=mouse=fogmap=0;
	collisionmode=0;
	excludecollision=0;
	filter.vec(0,0,0,1);
	shadercolor.vec(1,1,1,1);
	shadowcolor.vec(0.6f,0.6f,0.6f,1);
	bsppicflipy=0;
	flyRender::s_fog=0;
	flyRender::s_background[0]=
	flyRender::s_background[1]=
	flyRender::s_background[2]=0;
	flyRender::s_clearbg=1;

	bspfile="";
	status_msg_time=0;
	T0=0;
	T1=0;
	force_dt=0;

	cam=0;
	player=0;
}

int flyEngine::step()
{	
	int dt;

	if (force_dt)
	{
		cur_time=T0+force_dt;
		cur_time_float=cur_time/1000.0f;
		T0=cur_time;
		dt=force_dt;
		step(dt);
		frame_count++;
		if (cur_time-T1>500)
		{
			frame_rate=frame_count*1000/(cur_time-T1);
			T1=cur_time;
			frame_count=0;
		}
		if (g_flyrender)
			g_flyrender->m_curtime=cur_time;
		return dt;
	}

	cur_time=timeGetTime()-start_time;
	cur_time_float=cur_time/1000.0f;

	dt=cur_time-T0;

	if (dt>0)
	{
		T0=cur_time;
		if (dt<1000)
		{
			step(dt);
			frame_count++;

			if (cur_time-T1>500)
			{
				frame_rate=frame_count*1000/(cur_time-T1);
				T1=cur_time;
				frame_count=0;
			}
			
			if (g_flyrender)
				g_flyrender->m_curtime=cur_time;

			return dt;
		}
	}

	if (g_flyrender)
		g_flyrender->m_curtime=cur_time;

	return 0;
}

void flyEngine::step(int dt)
{
	// process console commands
	if (console_command[0])
		{
		int p;
		flyString str=console_command;
		flyString str2;
		do	{
			p=str.find(';');
			if (p==-1)
				p=str.length();
			con.command_exec(str.left(p));
			if (str.length()-p-1>0)
				{
				str2.copy(str,p+1,str.length()-p-1);
				str=str2;
				}
			else 
				str="";
			} while(str.length());
		}
	console_command="";

	poststeps.clear();

	// if not in server mode
	if (g_flydirectx->mpmode!=FLY_MP_SERVER)
		step_init(dt);

	if (player!=0 && cam!=0 && bsp!=0)
		step_objects(dt);

	// if a texture cache is available and not in server mode
	if (g_flydirectx->mpmode!=FLY_MP_SERVER)
		step_end(dt);

	// if in multiplayer, check multiplayer messages
	if (g_flydirectx->mpmode!=FLY_MP_NOMP)
		check_multiplayer();

	// if in client multiplayer mode
	if (g_flydirectx->mpmode==FLY_MP_CLIENT && demo_file_in==0)
		{
		// update client objects to server at slower intervals
		static int last_mp_update=0;
		if (cur_time-last_mp_update>mpdelay)
			{
			last_mp_update=cur_time;
			dll.send_message(FLY_MESSAGE_MPUPDATE,0,0);
			}
		}
	
	// step all running plugins
	dll.send_message(FLY_MESSAGE_UPDATESCENE,dt,0);

	// call all post_step() required objects
	for( int i=0;i<poststeps.num;i++ )
		poststeps[i]->post_step();

	// step console
	if (con.mode)
		con.step(dt);
}

void flyEngine::step_init(int dt)
{
	if (g_flydirectx==0)
		return;
	
	// get input
	if (noinput)
		g_flydirectx->zero_input();
	else g_flydirectx->get_input();
	if (mastervolume<=0)
		g_flydirectx->set_master_volume(mastervolume);

	if (g_flytexcache==0)
		return;

	// attebuate filter
	filter.x-=dt/500.0f;
	filter.y-=dt/500.0f;
	filter.z-=dt/500.0f;
	if (filter.x<0) filter.x=0;
	if (filter.x>1) filter.x=1;
	if (filter.y<0) filter.y=0;
	if (filter.y>1) filter.y=1;
	if (filter.z<0) filter.z=0;
	if (filter.z>1) filter.z=1;

	int i;
	flyLightMap *map;

	if (lm.num)
	for( i=0;i<lmchanged.num;i++ )
	{
		map=lm[lmchanged[i]];
		map->load(lmpic[map->pic]);
		g_flytexcache->update_subtex(
			map->pic+lmbase,map->offsetx,map->offsety,
			map->sizex,map->sizey,3,map->bmp);
	}

	if (fm.num && fogmap)
	for( i=0;i<fmchanged.num;i++ )
	{
		map=fm[fmchanged[i]];
		memset(map->bmp,0,map->bytesxy);
		g_flytexcache->update_subtex(
			map->pic+fmbase,map->offsetx,map->offsety,
			map->sizex,map->sizey,4,map->bmp);
	}

	int j;
	flyFace *f;
	for( i=0;i<facechanged.num;i++ )
	{
		f=&faces[facechanged[i]];
		for( j=0;j<f->nvert;j++ )
			vert[j+f->vertindx].color=vert[j+f->vertindx].color_static;
	}

	facechanged.clear();
	lmchanged.clear();
	fmchanged.clear();
}

void flyEngine::step_end(int dt)
{
	if (g_flytexcache==0)
		return;

	int i;
	flyLightMap *map;

	if (lm.num)
	for( i=0;i<lmchanged.num;i++ )
		{
		map=lm[lmchanged[i]];
		g_flytexcache->update_subtex(
			map->pic+lmbase,map->offsetx,map->offsety,
			map->sizex,map->sizey,3,map->bmp);
		}

	if (fm.num && fogmap)
	for( i=0;i<fmchanged.num;i++ )
		{
		map=fm[fmchanged[i]];
		g_flytexcache->update_subtex(
			map->pic+fmbase,map->offsetx,map->offsety,
			map->sizex,map->sizey,4,map->bmp);
		}
}

void flyEngine::step_objects(int dt)
{
	cur_step_base=cur_step+1;
	cur_dt=dt;

	if (player && player->source)
	{
		cur_step++;
		if (player->step(dt))
			if (player)
				add_to_bsp(player);
	}

	if (cam && cam->source && cam!=player)
	{
		cur_step++;
		if (cam->step(dt))
			if (cam)
				add_to_bsp(cam);
	}

	// loop all active objects
	flyBspObject *o=(flyBspObject *)active_objs.get_next_object(0);
	while(o)
	{
		if (o!=cam && o!=player)
		{
			// step object and reposition in bsp if needed
			cur_step++;
			if (o->step(dt))
				add_to_bsp(o);
			
			// if object life is negative, destroy it
			if (o->life<0)
			{
				flyBspObject *tmp=o;
				o=(flyBspObject *)active_objs.get_next_object(o);
				active_objs.remove(tmp);
				tmp->remove_from_bsp();
				delete tmp;
				continue;
			}
		}
		o=(flyBspObject *)active_objs.get_next_object(o);
	}
}

void flyEngine::activate(flyBspObject *obj)
{
	if (obj==0) 
		return;
	
	active_objs.add(obj);

	if (loading==0)
	{
		obj->init();
		add_to_bsp(obj);
	}
}

void flyEngine::add_to_bsp(flyBspObject *obj)
{
	obj->remove_from_bsp();
	recurse_bsp_box(obj->pos+obj->bbox.min,obj->pos+obj->bbox.max,-1);
	flyBspNode *eye=find_node(obj->pos);
	int i;
	if (eye)
	{
		for( i=0;i<selnodes.num;i++ )
			if (selnodes[i]==eye)
				break;
		if (i<selnodes.num)
		{
			selnodes[i]=selnodes[0];
			selnodes[0]=eye;
		}
	}
	for( i=0;i<selnodes.num;i++ )
	{
		obj->clipnodes.add(selnodes[i]);
		selnodes[i]->elem.add(obj);
	}
}

void flyEngine::console_printf(const char *fmt, ...)
{
    static char ach[1024];

    va_list va;
    va_start( va, fmt );
    vsprintf( ach, fmt, va );
    va_end( va );

	status_msg=ach;
	status_msg_time=cur_time;

	con.add_string(ach);
}

void flyEngine::set_camera(flyBspObject *d)
{
	cam=d;
	if (cam)
		{
		view.build(cam->pos,cam->X,cam->Y,cam->Z);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(flyRender::s_camangle,flyRender::s_aspect,flyRender::s_nearplane,flyRender::s_farplane);
		if (flyRender::s_fog==0)
			glMultMatrixf((float *)&cam->mat_t);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (flyRender::s_fog)
			glMultMatrixf((float *)&cam->mat_t);
		glTranslatef(-cam->pos.x,-cam->pos.y,-cam->pos.z);

		glGetDoublev(GL_PROJECTION_MATRIX,cam_proj_mat);
		glGetDoublev(GL_MODELVIEW_MATRIX,cam_model_mat);
		glGetIntegerv(GL_VIEWPORT,cam_viewport);
		}
}

void flyEngine::close_fly_file()
{
	dll.send_message(FLY_MESSAGE_CLOSESCENE,0,0);
	if(bsp)
		close_multiplayer();
	if (g_flyrender)
	{
		g_flytexcache->reset();
		load_default_tex(flysdkdatapath);
	}
	reset();
}

int flyEngine::open_fly_file(const char *file)
{
	flyString str;
	
	if(bsp)
		close_multiplayer();

	close_fly_file();

	str=file;
	str.lower();

	flyfile=str;

	flyfilepath=flysdkdatapath;

	flyfilename=flyfilepath;
	flyfilename+=flyfile;

	int p=str.find_reverse('\\');
	if (p==-1) p=str.find_reverse('/');
	if (p!=-1) 
	{ 
		str.set_char(p+1,0);
		flyfilepath+=str;
	}

	if (load_data()==0)
	{
		reset();
		console_printf("Error loading map!");
		return 0;
	}

	return 1;
}

int flyEngine::save_fly_file(const char *file) 
{
	flyString str;

	str=file;
	str.lower();
	flyfile=str;
	flyfilepath=flysdkdatapath;
	flyfilename=flyfilepath;
	flyfilename+=flyfile;
	
	int p=str.find_reverse('\\');
	if (p==-1) p=str.find_reverse('/');
	if (p!=-1) 
	{ 
		str.set_char(p+1,0);
		flyfilepath+=str;
	}

	FILE *fp=fopen(flyfilename,"wt");
	if (fp==0) 
	{
		flyfilepath="";
		flyfilename="";
		flyfile="";
		return 0;
	}
	
	const char *name;
	flyString str1,str2;
	int i,j,n,type;
	flyBspObject *o,*defaultobj;
	flyParamDesc pd;

	fprintf(fp,"[classes]\n");
	for( i=0;i<dll.ndll;i++ )
		fprintf(fp,"dll%i=%s\n",i,(const char *)dll.dll[i]->dll_filename);

	for( i=0;i<dll.ndll;i++ )
	{
		if (dll.dll[i]->get_global_param_desc)
		{
			n=dll.dll[i]->get_global_param_desc(0,0);
			if (n)
			{
			fprintf(fp,"\n[%s]\n",(const char *)dll.dll[i]->dll_filename);
			for( j=0;j<n;j++ )
				{
				dll.dll[i]->get_global_param_desc(j,&pd);
				fprintf(fp,"%s=%s\n",(const char *)pd.name,pd.get_string());
				}
			}
		}
	}

	for( i=0;i<dll.ncd;i++ )
	{
		name=dll.cd[i]->get_name();
		type=dll.cd[i]->get_type();
		o=(flyBspObject *)stock_objs.get_next_object(0,type);
		n=0;
		while(o)
			{
			o->name.format("%s%i",name,n++);
			o=(flyBspObject *)o->next_obj;
			}
	}

	o=(flyBspObject *)active_objs.get_next_object(0);
	while(o)
	{
		if (o->source)
			o->name=o->source->name;
		o=(flyBspObject *)active_objs.get_next_object(o);
	}

	for( i=0;i<dll.ncd;i++ )
	{
		name=dll.cd[i]->get_name();
		type=dll.cd[i]->get_type();
		defaultobj=dll.cd[i]->create();
		o=(flyBspObject *)stock_objs.get_next_object(0,type);
		n=0;
		while(o)
			{
			if (o->type==type)
				{
				fprintf(fp,"\n");
				fprintf(fp,"[%s]\nlongname=%s\n",(const char *)o->name,(const char *)o->long_name);
				n=o->get_param_desc(0,0);
				for( j=0;j<n;j++ )
					{
					defaultobj->get_param_desc(j,&pd);
					str1=pd.get_string();
					o->get_param_desc(j,&pd);
					str2=pd.get_string();
					if (str1.compare_nocase(str2))
						fprintf(fp,"%s=%s\n",(const char *)pd.name,(const char *)str2);
					}
				}
			o=(flyBspObject *)o->next_obj;
			}
		delete defaultobj;
	}

	fprintf(fp,"\n");
	fprintf(fp,"[fly3d]\n");
	n=get_global_param_desc2(0,0);
	for( i=0;i<n;i++ )
	{
		get_global_param_desc2(i,&pd);
		if (pd.type!=' ')
			fprintf(fp,"%s=%s\n",(const char *)pd.name,pd.get_string());
	}

	fclose(fp);
	return 1;
}

void flyEngine::add_post_step(flyBspObject *o)
{
	poststeps.add(o);	
}

int flyEngine::lock_input(int free)
{
	if (free==1)
		noinput=0;
	else
	if (free==0 && noinput==0)
	{
		noinput=1;
		return 1;
	}
	return -1;
}