#include "../Fly3D.h"

flyDll::flyDll() :
	hdll(0),nclasses(0),
	num_classes(0),
	fly_message(0),
	get_class_desc(0),
	get_global_param_desc(0)
{
}

flyDll::~flyDll()
{ 
	if (hdll)
		FreeLibrary(hdll);
}

flyDllGroup::flyDllGroup() :
	dll(0),ndll(0),cd(0),ncd(0)
{
}

flyDllGroup::~flyDllGroup()
{ 
	reset(); 
}

void flyDllGroup::reset()
{
	for(int i=0;i<ndll;i++ ) 
		delete dll[i];

	delete[] dll;
	dll=0;
	ndll=0;

	delete[] cd;
	cd=0; 
	ncd=0;
}

void flyDllGroup::load_default_param(flyBspObject *o,const char *sec,flyFile *file)
{
	o->load_default_params(file,sec);
	o->rotate(o->rot);

	g_flyengine->stock_objs.add(o);

	int i;
	for( i=0;i<o->active;i++ )
		g_flyengine->activate(o->clone());
}

void flyDllGroup::load_all_classes(flyFile *file)
{
	for( int d=0;d<ndll;d++ )
	{
		load_params(d,file);
		load_classes(d,file);
	}
}

void flyDllGroup::load_params(int d,flyFile *file)
{
	if (dll[d]->get_global_param_desc)
	{
		flyParamDesc pd;
		flyString ret;
		int i,j=dll[d]->get_global_param_desc(0,0);
		for( i=0;i<j;i++ )
		{
			dll[d]->get_global_param_desc(i,&pd);
			ret="";
			if (file->get_profile_string(dll[d]->dll_filename,pd.name,ret))
				pd.set_string(ret);

		}
	}
}

void flyDllGroup::load_classes(int d,flyFile *file)
{
	if (dll[d]->num_classes && dll[d]->get_class_desc)
	{
	int i,j;
	dll[d]->nclasses=dll[d]->num_classes();
	flyClassDesc **tmp=new flyClassDesc *[ncd+dll[d]->nclasses];
	if (cd) 
		{
		memcpy(tmp,cd,sizeof(flyClassDesc *)*ncd);
		delete[] cd;
		}
	cd=tmp;
	for( i=0;i<dll[d]->nclasses;i++ )
		cd[i+ncd]=dll[d]->get_class_desc(i);

	if (file)
	{
	flyBspObject *o;
	flyString str,ret;
	for( i=0;i<dll[d]->nclasses;i++ )
		{
		j=0;
		while(1)
			{
			str.format("%s%i",cd[i+ncd]->get_name(),j++);
			ret="";
			file->get_profile_string(str,"longname",ret);
			if (ret[0]==0) break;
			o=cd[i+ncd]->create();
  			o->name=str;
			o->long_name=ret;
			load_default_param(o,str,file);
			}
		}
	}
	
	ncd+=dll[d]->nclasses;
	}
}

int flyDllGroup::send_message(int msg,int param,void *data) const 
{
	int i;
	for( i=0;i<ndll;i++ )
		if (dll[i]->fly_message(msg,param,data)==0)
			return 0;
	return 1;
}

int flyDllGroup::add_dll(const char *filename)
{
	flyString file=g_flyengine->flysdkpluginpath+filename;
	HINSTANCE d=LoadLibrary(file);
	if (d)
	{
		if (GetProcAddress(d,"num_classes")==0 ||
			GetProcAddress(d,"get_class_desc")==0 ||
			GetProcAddress(d,"fly_message")==0) 
			{
			g_flyengine->console_printf("%s: Not a Fly3D plugin",filename);
			return -1;
			}

		flyDll **tmp=new flyDll *[ndll+1];
		memcpy(tmp,dll,sizeof(flyDll *)*ndll);
		delete[] dll;
		dll=tmp;
		
		dll[ndll]=new flyDll;
		dll[ndll]->hdll=d;
		dll[ndll]->dll_filename=filename;
		dll[ndll]->num_classes=(int (*)())GetProcAddress(d,"num_classes");
		dll[ndll]->fly_message=(int (*)(int msg,int param,void *data))GetProcAddress(d,"fly_message");
		dll[ndll]->get_version=(int (*)())GetProcAddress(d,"get_version");
		dll[ndll]->get_class_desc=(flyClassDesc *(*)(int i))GetProcAddress(d,"get_class_desc");
		dll[ndll]->get_global_param_desc=(int (*)(int i,flyParamDesc *pd))GetProcAddress(d,"get_global_param_desc");
		dll[ndll]->nclasses=0;

		return ndll++;
	} 
	else 
	{ 
		g_flyengine->console_printf("%s: Failed to open",filename);
		return -1;
	}
}

int flyDllGroup::delete_dll(const char *filename)
{
	int i,j=0,k;
	
	for( i=0;i<ndll;i++ )
		if (!strcmp(filename,dll[i]->dll_filename))
			break;
		else j+=dll[i]->nclasses;
	if (i==ndll)
		return 0;
	
	for( k=j;k<j+dll[i]->nclasses;k++ )
	{
		int type=cd[k]->get_type();
		flyBspObject *o=(flyBspObject *)g_flyengine->stock_objs.get_next_object(0,type);
		while(o)
		{
			flyBspObject *tmp=o;
			o=(flyBspObject *)o->next_obj;
			delete_class(tmp);
		}
	}

	memcpy(&cd[j],&cd[j+dll[i]->nclasses],(ncd-(j+dll[i]->nclasses))*sizeof(flyClassDesc *));
	ncd-=dll[i]->nclasses;
	delete dll[i];
	memcpy(&dll[i],&dll[i+1],(ndll-(i+1))*sizeof(flyDll *));
	ndll--;
	return 1;
}

void flyDllGroup::delete_references(flyBspObject *o)
{
	flyParamDesc pd;
	int n,k;
	flyBspObject *obj;
	
	obj=(flyBspObject *)g_flyengine->active_objs.get_next_object(0);
	while(obj)
	{
		n=obj->get_param_desc(0,0);
		for( k=0;k<n;k++ )
			{
			obj->get_param_desc(k,&pd);
			if (pd.type>255 || pd.type=='d' || pd.type=='o')
				if ((*((flyBspObject **)pd.data))==o)
					*((flyBspObject **)pd.data)=0;
			}
		
		obj=(flyBspObject *)g_flyengine->active_objs.get_next_object(obj);
	}

	obj=(flyBspObject *)g_flyengine->stock_objs.get_next_object(0);
	while(obj)
	{
		n=obj->get_param_desc(0,0);
		for( k=0;k<n;k++ )
			{
			obj->get_param_desc(k,&pd);
			if (pd.type>255 || pd.type=='d' || pd.type=='o')
				if ((*((flyBspObject **)pd.data))==o)
					*((flyBspObject **)pd.data)=0;
			}
		
		obj=(flyBspObject *)g_flyengine->active_objs.get_next_object(obj);
	}
}

void flyDllGroup::delete_class(flyBspObject *obj)
{
	flyBspObject *o,*tmp;
	
	o=(flyBspObject *)g_flyengine->active_objs.get_next_object(0,obj->type);
	while(o)
	{
		if (o->source==obj)
		{
			if (g_flyengine->cam==o)
				g_flyengine->cam=0;
			if (g_flyengine->player==o)
				g_flyengine->player=0;
			tmp=o;
			o=(flyBspObject *)o->next_obj;
			g_flyengine->active_objs.remove(tmp);
			delete_references(tmp);
			tmp->remove_from_bsp();
			delete tmp;
		}
		else
			o=(flyBspObject *)o->next_obj;
	}

	if (g_flyengine->cam==obj)
		g_flyengine->cam=0;
	if (g_flyengine->player==obj)
		g_flyengine->player=0;
	g_flyengine->stock_objs.remove(obj);
	delete_references(obj);
	delete obj;
}

flyBspObject *flyDllGroup::add_class(const char *name)
{
	int i;
	flyBspObject *o=0;
	for( i=0;i<ncd;i++ )
		if (!stricmp(name,cd[i]->get_name()))
			break;
	if (i<ncd)
	{
		o=cd[i]->create();
		g_flyengine->stock_objs.add(o);
	}
	return o;
}

void flyDllGroup::move_dll(const char *filename,int newrelpos)
{
	int i,j,k;
	for( i=0;i<ndll;i++ )
		if (!strcmp(filename,dll[i]->dll_filename))
			break;
	if (i<ndll)
	{
		flyDll *d=dll[i];
		dll[i]=dll[i+newrelpos];
		dll[i+newrelpos]=d;

		k=0;
		for( i=0;i<ndll;i++ )
			for( j=0;j<dll[i]->nclasses;j++ )
				cd[k++]=dll[i]->get_class_desc(j);
	}
}
