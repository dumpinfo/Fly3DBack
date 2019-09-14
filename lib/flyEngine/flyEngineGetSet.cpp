#include "../Fly3D.h"

int flyEngine::get_picture(const char *file,int droplevel)
{
	if (file[0]==0 || g_flytexcache==0) 
		return -1;

	int i=g_flytexcache->find_tex(file);
	if (i!=-1)
		return i;

	flyFile fp;
	int ret=0;
	int flipy=(loading && bsppicflipy)?0:1;

	flyString name=flyfilepath+"maps\\"+file;
	int pos=name.find('.');
	if (pos!=-1 && name.length()-pos!=4)
		pos=-1;

	if (pos==-1)
	{
		flyPicture *p=new flyPicture;
		strcpy(p->name,file);

		flyString str=name+".tga";
		if (fp.open(str))
			ret=p->LoadTGA(fp.buf,fp.len,flipy);
		if (ret==0)
			{
			str=name+".jpg";
			if (fp.open(str))
				ret=p->LoadJPG(fp.buf,fp.len,flipy);
			}

		if (ret==0)
			ret=-1;
		else
		{
			p->CheckSize(droplevel);
			ret=g_flytexcache->add_tex(1,&p,3+(flyTexCache::s_textruecolor?4:0)+(flyTexCache::s_texcompress?16:0));
		}

		delete p;
	}
	else
	{
		flyString s=name.right(3);
		if (s.compare_nocase("avi")==0)
		{
			flyVideo *v=new flyVideo;
			if (v->load_avi(name))
			{
				ret=v->texture;
				strcpy(g_flytexcache->texinfo[ret].name,file);
			}
			else
			{
				ret=-1;
				delete v;
			}
		}
		else
		{
			flyPicture *p=new flyPicture;
			strcpy(p->name,file);

			if (s.compare_nocase("tga")==0)
			{
				if (fp.open(name))
					ret=p->LoadTGA(fp.buf,fp.len,flipy);
			}
			else
			if (s.compare_nocase("jpg")==0)
			{
				if (fp.open(name))
					ret=p->LoadJPG(fp.buf,fp.len,flipy);
			}

			if (ret==0)
				ret=-1;
			else
			{
				p->CheckSize(droplevel);
				ret=g_flytexcache->add_tex(1,&p,3+(flyTexCache::s_textruecolor?4:0)+(flyTexCache::s_texcompress?16:0));
			}

			delete p;
		}
	}

	return ret;
}

int flyEngine::get_input_map(const char *name)
{
	int i;
	for( i=0;i<inputmaps.num;i++ )
		if (inputmaps[i].name.compare_nocase(name)==0)
			return i;
	return -1;
}

int flyEngine::check_input_map(int i,int clicked)
{
	if(noinput || i<0 || i>=inputmaps.num)
		return false;

	if(clicked)
		return inputmaps[i].check_clicked_input();

	return inputmaps[i].check_input();
}

flyBezierCurve *flyEngine::get_bezier_curve(const char *name)
{
	if (name[0]==0) 
		return 0;

	flyBezierCurve *o=(flyBezierCurve *)resource_objs.get_object_name(name,FLY_TYPE_BEZIERCURVE);

	if (o==0)
	{
		o=new flyBezierCurve;
		o->name=name;
		o->long_name=name;

		flyString str=flyfilepath+"objects\\"+name;
		if (o->load_bez(str)==0)
			{
			delete o;
			return 0;
			}

		resource_objs.add(o);
	}

	return o;
}

flySound *flyEngine::get_sound_object(const char *name)
{
	if (name[0]==0) 
		return 0;

	flySound *s=(flySound *)resource_objs.get_object_name(name,FLY_TYPE_SOUND);
	
	if (s==0)
	{
		s=new flySound;
		s->name=name;
		s->long_name=name;
	
		flyString str=flyfilepath+"sounds\\"+name;
		if (s->load_wav(str)==0)
		{
			delete s;
			return 0;
		}

		resource_objs.add(s);
	}

	return s;
}

flyMesh *flyEngine::get_model_object(const char *name)
{
	if (name[0]==0) 
		return 0;

	flyMesh *o;
	o=(flyMesh *)resource_objs.get_object_name(name,FLY_TYPE_MESH);
	if (o==0)
		o=(flyMesh *)resource_objs.get_object_name(name,FLY_TYPE_VERTEXMESH);
	if (o==0)
		o=(flyMesh *)resource_objs.get_object_name(name,FLY_TYPE_SKELETONMESH);

	if (o==0)
	{
		flyString str;
		str=flyfilepath+"objects\\"+name;
	
		int type,ver;
		flyFile fp;
		if (fp.open(str)==0)
			return 0;
		fp.read(&type,sizeof(int));
		fp.read(&ver,sizeof(int));
		fp.close();

		switch(type)
		{
		case FLY_FILEID_F3D: 
			o=new flyAnimatedMesh;
			if (o->load(str)==0)
				{
				delete o;
				return 0;
				}
			break;
		case FLY_FILEID_K3D: 
			o=new flySkeletonMesh;
			if (o->load(str)==0)
				{
				delete o;
				return 0;
				}
			break;
		}

		o->name=name;
		o->long_name=name;
		
		int pos=str.find('.');
		if (pos!=-1)
			str.set_char(pos,0);
		str+=".shr";
		for( int i=0;i<o->nf;i++ )
			o->localfaces[i].sh+=shaders.num;
		load_shaders(str);

		resource_objs.add(o);
	}

	return o;
}

int flyEngine::get_obj_param(const char *objname,const char *param,flyString& value) const
{
	value="";

	flyBspObject *obj;
	obj=(flyBspObject *)active_objs.get_object_longname(objname);
	if (obj==0)
		obj=(flyBspObject *)stock_objs.get_object_longname(objname);
	if (obj==0) 
		return 1;
	
	int i,n;
	flyParamDesc pd;
	n=obj->get_param_desc(0,0);
	for( i=0;i<n;i++ )
		{
		obj->get_param_desc(i,&pd);
		if (!stricmp(pd.name,param))
			break;
		}
	if (i==n)
		return 2;

	value=pd.get_string();

	return 0;
}

int flyEngine::get_global_param_desc0(int num,flyParamDesc *pd) 
{
	int i,j,k;
	j=get_global_param_desc1(0,0)+get_global_param_desc2(0,0);
	for( i=0;i<dll.ndll;i++ )
		if (dll.dll[i]->get_global_param_desc)
			j+=dll.dll[i]->get_global_param_desc(0,0);

	if (pd==0)
		return j;

	j=0;
	i=get_global_param_desc1(0,0);
	if (num<i)
		get_global_param_desc1(num,pd);
	else
	{
		j=i;
		i+=get_global_param_desc2(0,0);
		if (num<i)
			get_global_param_desc2(num-j,pd);
		else
		{
			for( k=0;k<dll.ndll;k++ )
				if (dll.dll[k]->get_global_param_desc)
				{
					j=i;
					i+=dll.dll[k]->get_global_param_desc(0,0);
					if (num<i)
						{
						dll.dll[k]->get_global_param_desc(num-j,pd);
						break;
						}
				}
		}
	}

	return j;
}

int flyEngine::get_global_param_desc1(int i,flyParamDesc *pd) 
{
	if (pd!=0)
	switch(i)
	{
		case 0:
			pd->type='s';
			pd->data=&playername;
			pd->name="playername";
			break;
		case 1:
			pd->type='c';
			pd->data=&playercolor;
			pd->name="playercolor";
			break;
		case 2:
			pd->type=-'y';
			pd->data=&inputmaps;
			pd->name="inputmaps";
			break;
		case 3:
			pd->type=' ';
			pd->data=0;
			pd->name="";
			break;
		case 4:
			pd->type='i';
			pd->data=&debug;
			pd->name="debug";
			break;
		case 5:
			pd->type='i';
			pd->data=&sse;
			pd->name="sse";
			break;
		case 6:
			pd->type='i';
			pd->data=&octree;
			pd->name="octree";
			break;
		case 7:
			pd->type='i';
			pd->data=&vertprog;
			pd->name="vertprog";
			break;
		case 8:
			pd->type='f';
			pd->data=&curveerr;
			pd->name="curveerr";
			break;
		case 9:
			pd->type='f';
			pd->data=&geomdetail;
			pd->name="geomdetail";
			break;
		case 10:
			pd->type='i';
			pd->data=&flyRender::s_texdroplevel;
			pd->name="texdroplevel";
			break;
		case 11:
			pd->type='i';
			pd->data=&flyRender::s_videores;
			pd->name="videores";
			break;

		case 12:
			pd->type=' ';
			pd->data=0;
			pd->name="";
			break;

		case 13:
			pd->type='i';
			pd->data=&flyRender::s_antialias;
			pd->name="antialias";
			break;
		case 14:
			pd->type='i';
			pd->data=&multitexture;
			pd->name="multitexture";
			break;
		case 15:
			pd->type='i';
			pd->data=&pvsoff;
			pd->name="pvsoff";
			break;
		case 16:
			pd->type='i';
			pd->data=&nodeonly;
			pd->name="nodeonly";
			break;
		case 17:
			pd->type='i';
			pd->data=&status;
			pd->name="status";
			break;
		case 18:
			pd->type='i';
			pd->data=&flyRender::s_stencil;
			pd->name="stencil";
			break;
		case 19:
			pd->type='i';
			pd->data=&flyTexCache::s_texfilter;
			pd->name="texfilter";
			break;
		case 20:
			pd->type='i';
			pd->data=&flyTexCache::s_texmipmap;
			pd->name="texmipmap";
			break;
		case 21:
			pd->type='i';
			pd->data=&flyTexCache::s_textruecolor;
			pd->name="textruecolor";
			break;
		case 22:
			pd->type='i';
			pd->data=&flyTexCache::s_texcompress;
			pd->name="texcompress";
			break;
		case 23:
			pd->type='i';
			pd->data=&flyRender::s_wireframe;
			pd->name="wireframe";
			break;
		case 24:
			pd->type=' ';
			pd->data=0;
			pd->name="";
			break;
		case 25:
			pd->type='i';
			pd->data=&shadows;
			pd->name="shadows";
			break;
		case 26:
			pd->type='i';
			pd->data=&shadermask;
			pd->name="shadermask";
			break;
		case 27:
			pd->type='i';
			pd->data=&mpdelay;
			pd->name="mpdelay";
			break;
		case 28:
			pd->type='i';
			pd->data=&mute;
			pd->name="mute";
			break;
		case 29:
			pd->type='i';
			pd->data=&mastervolume;
			pd->name="mastervolume";
			break;
		case 30:
			pd->type=' ';
			pd->data=0;
			pd->name="";
			break;
		case 31:
			pd->type='f';
			pd->data=&flyDirectx::s_mousespeed;
			pd->name="mousespeed";
			break;
		case 32:
			pd->type='i';
			pd->data=&flyDirectx::s_invertmousex;
			pd->name="invertmousex";
			break;
		case 33:
			pd->type='i';
			pd->data=&flyDirectx::s_invertmousey;
			pd->name="invertmousey";
			break;
#ifndef FLY_WIN_NT4_SUPORT
		case 34:
			pd->type='f';
			pd->data=&flyDirectx::s_joyspeed;
			pd->name="joyspeed";
			break;
		case 35:
			pd->type='i';
			pd->data=&flyDirectx::s_invertjoyx;
			pd->name="invertjoyx";
			break;
		case 36:
			pd->type='i';
			pd->data=&flyDirectx::s_invertjoyy;
			pd->name="invertjoyy";
			break;
	}
	return 37;
#else
	}
	return 34;
#endif
}

int flyEngine::get_global_param_desc2(int i,flyParamDesc *pd) 
{
	if (pd!=0)
	switch(i)
	{
		case 0:
			pd->type='b';
			pd->data=&bspfile;
			pd->name="bspfile";
			break;
		case 1:
			pd->type='f';
			pd->data=&flyRender::s_nearplane;
			pd->name="nearplane";
			break;
		case 2:
			pd->type='f';
			pd->data=&flyRender::s_farplane;
			pd->name="farplane";
			break;
		case 3:
			pd->type='f';
			pd->data=&flyRender::s_camangle;
			pd->name="camangle";
			break;
		case 4:
			pd->type='d';
			pd->data=&cam;
			pd->name="camera";
			break;
		case 5:
			pd->type='d';
			pd->data=&player;
			pd->name="player";
			break;
		case 6:
			pd->type='f';
			pd->data=&gravity;
			pd->name="gravity";
			break;

		case 7:
			pd->type=' ';
			pd->data=0;
			pd->name="";
			break;

		case 8:
			pd->type='c';
			pd->data=&flyRender::s_background;
			pd->name="background";
			break;
		case 9:
			pd->type='i';
			pd->data=&flyRender::s_clearbg;
			pd->name="clearbg";
			break;
		case 10:
			pd->type='p';
			pd->data=&crosshairpic;
			pd->name="crosshairpic";
			break;
		case 11:
			pd->type='i';
			pd->data=&crosshairsize;
			pd->name="crosshairsize";
			break;
		case 12:
			pd->type='i';
			pd->data=&dynlight;
			pd->name="dynlight";
			break;
		case 13:
			pd->type='i';
			pd->data=&flyRender::s_fog;
			pd->name="fog";
			break;
		case 14:
			pd->type='i';
			pd->data=&fogmap;
			pd->name="fogmap";
			break;
		case 15:
			pd->type='i';
			pd->data=&mouse;
			pd->name="mouse";
			break;
		case 16:
			pd->type='i';
			pd->data=&bsppicflipy;
			pd->name="bsppicflipy";
			break;
		case 17:
			pd->type='c';
			pd->data=&shadowcolor;
			pd->name="shadowcolor";
			break;
		case 18:
			pd->type='f';
			pd->data=&shadowdist;
			pd->name="shadowdist";
			break;

		case 19:
			pd->type=' ';
			pd->data=0;
			pd->name="";
			break;

		case 20:
			pd->type='i';
			pd->data=&fraglimit;
			pd->name="fraglimit";
			break;
		case 21:
			pd->type='i';
			pd->data=&timelimit;
			pd->name="timelimit";
			break;
		case 22:
			pd->type='i';
			pd->data=&restarttime;
			pd->name="restarttime";
			break;
	}
	return 23;
}

int flyEngine::set_global_param(const char *name,const char *value)
{
	if (value[0]==0)
		return 0;

	flyParamDesc pd;
	int i,n1,n2;

	n1=get_global_param_desc1(0,0);
	n2=n1+get_global_param_desc2(0,0);
	for( i=0;i<n2;i++ )
		{
		if (i>=n1)
			get_global_param_desc2(i-n1,&pd);
		else
			get_global_param_desc1(i,&pd);
		if (!stricmp(name,pd.name))
			break;
		}
	if (i==n2)
	{
		for( n1=0;n1<dll.ndll;n1++ )
		{
			if (dll.dll[n1]->get_global_param_desc)
				n2=dll.dll[n1]->get_global_param_desc(0,0);
			else n2=0;
			for( i=0;i<n2;i++ )
			{
				dll.dll[n1]->get_global_param_desc(i,&pd);
				if (!stricmp(name,pd.name))
					break;
			}
			if (i<n2)
				break;
		}
		if (n1==dll.ndll)
			return 0;
	}
	
	pd.set_string(value);

	flyBspObject *obj;
	obj=(flyBspObject *)stock_objs.get_next_object(0);
	while(obj)
	{
		obj->message(flyVector(0,0,0),0,FLY_OBJMESSAGE_CHANGEPARAM,-i-1,&pd);
		obj=(flyBspObject *)stock_objs.get_next_object(obj);
	}
	obj=(flyBspObject *)active_objs.get_next_object(0);
	while(obj)
	{
		obj->message(flyVector(0,0,0),0,FLY_OBJMESSAGE_CHANGEPARAM,-i-1,&pd);
		obj=(flyBspObject *)active_objs.get_next_object(obj);
	}

	return 1;
}

int flyEngine::set_obj_param(const char *objname,const char *param,const char *value)
{
	if (value[0]==0)
		return 0;

	flyBspObject *obj=(flyBspObject *)stock_objs.get_object_longname(objname);
	if (obj==0) 
		return 1;

	flyParamDesc pd;
	int i,j,n;
	n=obj->get_param_desc(0,0);
	for( i=0;i<n;i++ )
		{
		obj->get_param_desc(i,&pd);
		if (!stricmp(pd.name,param))
			break;
		}
	if (i==n)
		return 2;

	j=4;
	if (pd.type=='d')
	{
		flyBspObject *o=(flyBspObject *)active_objs.get_object_longname(value);
		*((flyBspObject **)pd.data)=o;
	}
	else
	if (pd.type=='o' || pd.type>255)
	{
		flyBspObject *o=(flyBspObject *)stock_objs.get_object_longname(value);
		*((flyBspObject **)pd.data)=o;
	}
	else pd.set_string(value);

	switch(pd.type)
	{
	case 'i':
	case 'f':
	case 'a':
	case 'w':
	case 'd':
	case 'o':
	case 'z':
	case 'p':
	case 'm':
	case 'n':
	case 'k':
	case 'y':
		break;
	case 'v':
	case 'c':
		j=12;
		break;
	case 't':
		j=16;
	case 'b':
	case 's':
		j=-1;
		break;
	case -'c':
	case -'v':
	case -'t':
		j=-2;
		break;
	case -'y':
		j=-3;
	default:
		return 3;
	}

	obj->message(flyVector(0,0,0),0,FLY_OBJMESSAGE_CHANGEPARAM,0,0);
	char *data=(char *)pd.data;
	flyBspObject *o=(flyBspObject *)active_objs.get_next_object(0,obj->type);
	while(o)
		{
		if (o->source==obj)
			{
			o->get_param_desc(i,&pd);
			if (j==-1)
				*((flyString *)pd.data)=*((flyString *)data);
			else
			if (j==-2)
				*((flyArray<flyVector> *)pd.data)=*((flyArray<flyVector> *)data);
			else
			if (j==-3)
				*((flyArray<flyInputMap> *)pd.data)=*((flyArray<flyInputMap> *)data);
			else
				memcpy(pd.data,data,j);
			o->message(flyVector(0,0,0),0,FLY_OBJMESSAGE_CHANGEPARAM,i,&pd);
			}
		o=(flyBspObject *)o->next_obj;
		}
	
	return 0;
}
