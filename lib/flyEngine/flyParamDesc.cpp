#include "../Fly3D.h"

const char *flyParamDesc::get_string() const
{
	static flyString str;

	str.set_char(0,0);

	if (type>255)
		{
		if (*((flyBspObject **)data)!=0)
				str=(*((flyBspObject **)data))->name;
		}
	else
	if(type<-255 || type==-'o')
	{
		flyArray<flyBspObject*>& a=*((flyArray<flyBspObject*> *)data);
		for( int i=0;i<a.num;i++ )
		{
			if (i!=0) str+=" ";
			str+=a[i]->long_name;
		}
	}
	else
	switch(type)
	{
		case 'i':
			str.format("%i",*((int *)data));
			break;
		case -'i':
			{
			flyArray<int>& a=*((flyArray<int> *)data);
			flyString s;
			for( int i=0;i<a.num;i++ )
				{
				if (i!=0) str+=" ";
				s.format("%i",a[i]);
				str+=s;
				}
			}
			break;
		case 'f':
			str.format("%f",*((float *)data));
			break;
		case 'a':
			str.format("%f",acos(*((float *)data))*FLY_PIUNDER180);
			break;
		case -'f':
		case -'a':
			{
			flyArray<float>& a=*((flyArray<float> *)data);
			flyString s;
			for( int i=0;i<a.num;i++ )
				{
				if (i!=0) str+=" ";
				if (type==-'f')
					s.format("%f",a[i]);
				else
					str.format("%f",acos(a[i])*FLY_PIUNDER180);
				str+=s;
				}
			}
			break;
		case 'c':
		case 'v':
			str.format("%f %f %f",
				((float *)data)[0],
				((float *)data)[1],
				((float *)data)[2]);
			break;
		case -'c':
		case -'v':
			{
			flyArray<flyVector>& a=*((flyArray<flyVector> *)data);
			flyString s;
			for( int i=0;i<a.num;i++ )
				{
				if (i!=0) str+=" ";
				s.format("%f %f %f",a[i].x,a[i].y,a[i].z);
				str+=s;
				}
			}
			break;
		case 't':
			str.format("%f %f %f %f",
				((float *)data)[0],
				((float *)data)[1],
				((float *)data)[2],
				((float *)data)[3]);
			break;
		case -'t':
			{
			flyArray<flyVector>& a=*((flyArray<flyVector> *)data);
			flyString s;
			for( int i=0;i<a.num;i++ )
				{
				if (i!=0) str+=" ";
				s.format("%f %f %f %f",a[i].x,a[i].y,a[i].z,a[i].w);
				str+=s;
				}
			}
			break;
		case 'y':
			str.format("%s %i %i %i",	
				g_flyengine->inputmaps[*((int *)data)].name,
				g_flyengine->inputmaps[*((int *)data)].key,
				g_flyengine->inputmaps[*((int *)data)].mouse,
				g_flyengine->inputmaps[*((int *)data)].joystick);
			break;
		case -'y':
			{
			flyArray<flyInputMap>& a=*((flyArray<flyInputMap> *)data);
			flyString s;
			for( int i=0;i<a.num;i++ )
				{
				if (i!=0) str+=" ";
				s.format("%s %i %i %i",(const char *)a[i].name,a[i].key,a[i].mouse,a[i].joystick);
				str+=s;
				}
			}
			break;
		case 'p':
			if (*((int *)data)!=-1 && g_flytexcache)
				str=g_flytexcache->texinfo[*((int *)data)].name;
			break;
		case 'o':
			if (*((flyBspObject **)data)!=0)
				str=(*((flyBspObject **)data))->name;
			break;
		case 'd':
			if (*((flyBspObject **)data)!=0)
				str=(*((flyBspObject **)data))->name;
			break;
		case 'm':
			if (*((flyMesh **)data)!=0)
				str=(*((flyMesh **)data))->name;
			break;
		case 'n':
			if (*((flyAnimatedMesh **)data)!=0)
				str=(*((flyAnimatedMesh **)data))->name;
			break;
		case 'k':
			if (*((flySkeletonMesh **)data)!=0)
				str=(*((flySkeletonMesh **)data))->name;
			break;
		case 'z':
			if (*((flyBezierCurve **)data)!=0)
				str=(*((flyBezierCurve **)data))->name;
			break;
		case 'w':
			if (*((flySound **)data)!=0)
				str=(*((flySound **)data))->name;
			break;
		case 'b':
		case 's':
			str=*((flyString *)data);
			break;
		case -'s':
		{
			flyArray<flyString>& a=*((flyArray<flyString> *)data);
			for( int i=0;i<a.num;i++ )
			{
				if (i!=0) str+=" ";
				str+=a[i];
			}
		}
		break;
	}
	return str;
}

void flyParamDesc::set_string(const char *str)
{
	if (type>255)
		(*((flyBspObject **)data))=(flyBspObject *)g_flyengine->stock_objs.get_object_name(str);
	else
	if (type<-255)
	{
		char *c=(char *)str,*cc;
		flyArray<flyBspObject*> *a=(flyArray<flyBspObject*> *)data;
		flyBspObject *o;

		flyString s;
		a->clear();
		while(c && *c!=0)
		{
			cc=strchr(c,' ');
			if (cc==0)
			{
				if(o=(flyBspObject*)g_flyengine->stock_objs.get_object_longname(c,-type))
					a->add(o);
				break;
			}

			s.copy(c,0,cc-c);

			if(o=(flyBspObject*)g_flyengine->stock_objs.get_object_longname(s,-type))
				a->add(o);

			c=cc+1;
		}
	}
	else
	switch(type)
	{
		case 'i':
			*((int *)data)=0;
			sscanf(str,"%i",(int *)data);
			break;
		case -'i':
			{
				char *c=(char *)str;
				flyArray<int> *a=(flyArray<int> *)data;
				int i;
				a->clear();
				while(c && *c!=0)
				{
					if (sscanf(c,"%i",&i)!=1) 
						break;
					a->add(i);
					c=strchr(c,' ');
					if (c==0) break;
					c++;
				}
			}
			break;
		case 'f':
			*((float *)data)=0;
			sscanf(str,"%f",(float *)data);
			break;
		case 'a':
			*((float *)data)=0;
			sscanf(str,"%f",(float *)data);
			*((float *)data)=(float)cos(*((float *)data)*FLY_PIOVER180);
			break;
		case -'f':
		case -'a':
			{
				char *c=(char *)str;
				flyArray<float> *a=(flyArray<float> *)data;
				float f;
				a->clear();
				while(c && *c!=0)
				{
					if (sscanf(c,"%f",&f)!=1) 
						break;
					a->add(f);
					c=strchr(c,' ');
					if (c==0) break;
					c++;
				}
			}
			break;
		case 'c':
		case 'v':
			((float *)data)[0]=
			((float *)data)[1]=
			((float *)data)[2]=0;
			sscanf(str,"%f %f %f",
				&((float *)data)[0],
				&((float *)data)[1],
				&((float *)data)[2]);
			break;
		case -'c':
		case -'v':
			{
				char *c=(char *)str;
				flyArray<flyVector> *a=(flyArray<flyVector> *)data;
				flyVector v;
				a->clear();
				while(c && *c!=0)
				{
					for( int i=0;i<3; )
					{
						if (sscanf(c,"%f",&v[i])!=1) 
							break;
						else i++;
						c=strchr(c,' ');
						if (c==0) break;
						c++;
					}
					if (i==3)
						a->add(v);
					else break;
				}
			}
			break;
		case 't':
			((float *)data)[0]=
			((float *)data)[1]=
			((float *)data)[2]=
			((float *)data)[3]=0;
			sscanf(str,"%f %f %f %f",
				&((float *)data)[0],
				&((float *)data)[1],
				&((float *)data)[2],
				&((float *)data)[3]);
			break;
		case -'t':
			{
				char *c=(char *)str;
				flyArray<flyVector> *a=(flyArray<flyVector> *)data;
				flyVector v;
				a->clear();
				while(c && *c!=0)
				{
					for( int i=0;i<4; )
					{
						if (sscanf(c,"%f",&v[i])!=1) 
							break;
						else i++;
						c=strchr(c,' ');
						if (c==0) break;
						c++;
					}
					if (i==4)
						a->add(v);
					else break;
				}
			}
			break;
		case 'y':
			*((int *)data)=g_flyengine->get_input_map(str);
			break;
		case -'y':
			{
				char *c=(char *)str,*cc;
				flyArray<flyInputMap> *a=(flyArray<flyInputMap> *)data;
				flyInputMap i;
				a->clear();
				while(c && *c!=0)
				{
					cc=strchr(c,' ');
					if (cc==0) 
						break;
					i.name.copy(c,0,cc-c);
					c=cc+1;
					if (sscanf(c,"%i",&i.key)!=1) 
						break;
					c=strchr(c,' ');
					if (c==0) break;
					c++;
					if (sscanf(c,"%i",&i.mouse)!=1) 
						break;
					c=strchr(c,' ');
					if (c==0) break;
					c++;
					if (sscanf(c,"%i",&i.joystick)!=1) 
						break;
					a->add(i);
					c=strchr(c,' ');
					if (c==0) break;
					c++;
				}
			}
			break;
		case 'p':
			*((int *)data)=g_flyengine->get_picture(str);
			break;
		case 'o':
			*((flyBspObject **)data)=(flyBspObject *)g_flyengine->stock_objs.get_object_name(str);
			break;
		case -'o':
			{
				char *c=(char *)str,*cc;
				flyArray<flyBspObject*> *a=(flyArray<flyBspObject*> *)data;
				flyBspObject *o;

				flyString s;
				a->clear();
				while(c && *c!=0)
				{
					cc=strchr(c,' ');
					if (cc==0)
					{
						if(o=(flyBspObject*)g_flyengine->stock_objs.get_object_longname(c))
							a->add(o);
						break;
					}

					s.copy(c,0,cc-c);

					if(o=(flyBspObject*)g_flyengine->stock_objs.get_object_longname(s))
						a->add(o);

					c=cc+1;
				}
			}
			break;
		case 'd':
			*((flyBspObject **)data)=(flyBspObject *)g_flyengine->active_objs.get_object_name(str);
			break;
		case 'm':
			*((flyMesh **)data)=g_flyengine->get_model_object(str);
			break;
		case 'n':
			*((flyMesh **)data)=g_flyengine->get_model_object(str);
			break;
		case 'k':
			*((flyMesh **)data)=g_flyengine->get_model_object(str);
			break;
		case 'z':
			*((flyBezierCurve **)data)=g_flyengine->get_bezier_curve(str);
			break;
		case 'w':
			*((flySound **)data)=g_flyengine->get_sound_object(str);
			break;
		case 'b':
		case 's':
			*((flyString *)data)=str;
			break;
		case -'s':
		{
			char *c=(char *)str,*cc;
			flyArray<flyString> *a=(flyArray<flyString> *)data;

			flyString s;
			a->clear();
			while(c && *c!=0)
			{
				cc=strchr(c,' ');
				if (cc==0)
				{
					a->add(c);
					break;
				}
				s.copy(c,0,cc-c);
				a->add(s);
				c=cc+1;
			}
		}
		break;
	}
}
