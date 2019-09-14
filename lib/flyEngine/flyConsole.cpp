#include "../Fly3D.h"

flyConsole::flyConsole() :
	nlines(0),cmd_pos(0),cmd_len(0),mode(0),time(0),
	winlines(0),linecount(0),curr_line(-1),curr_page(1),lic(-1)
{
	memset(buf,0,sizeof(char *)*FLY_CONSOLE_MAXLINES);
	cmd_line[0]=0;
}

flyConsole::~flyConsole()
{
	int i;
	for( i=0;i<nlines;i++ )
		delete[] buf[i];
}

void flyConsole::step(int dt)
{
	time-=dt;
	if (time<0)
	{
		time=20;
		if (mode>0)
		{
			if (winlines<FLY_CONSOLE_LINE/3)
			{
				winlines++;
				while((curr_page-1)*(winlines-1)>=nlines && curr_page>1) curr_page--;
			}
		}
		else if (mode<0)
		{
			if (winlines>0)
			{
				winlines--;
				while((curr_page-1)*(winlines-1)>=nlines && curr_page>1) curr_page--;
			}
			if (winlines==0)
			{
				mode=0;
				g_flyengine->lock_input(lic);
			}
		}
	}
}

void flyConsole::show()
{
	lic=g_flyengine->lock_input();
	if (lic==-1)
		return;
	mode=1;
	cmd_line[cmd_pos=0]=0;
	cmd_len=0;
	curr_line=-1;
	curr_page=1;
}

void flyConsole::hide()
{
	mode=-1;
	cmd_line[cmd_pos=0]=0;
	cmd_len=0;
	curr_line=-1;
	curr_page=1;
}

void flyConsole::add_string(const char *fmt, ...)
{
    static char str[1024];

    va_list va;
    va_start( va, fmt );
    vsprintf( str, fmt, va );
    va_end( va );

	if (nlines==FLY_CONSOLE_MAXLINES)
		delete[] buf[--nlines];
	memmove(&buf[1],&buf[0],sizeof(char *)*nlines);
	int len=strlen(str);
	buf[0]=new char[len+1];
	memcpy(buf[0],str,len+1);
	nlines++;
	linecount++;
	
	if(curr_line!=-1)
	{
		curr_line++;
		if(curr_line==FLY_CONSOLE_MAXLINES)
			curr_line=-1;
	}
}

void flyConsole::draw()
{
	int i, j;
	float f1,f2,f3;

	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_BACK,GL_FILL);

	dx=(float)flyRender::s_screensizex/FLY_CONSOLE_COL;
	dy=(float)flyRender::s_screensizey/FLY_CONSOLE_LINE;

	f1=FLY_CONSOLE_LINE/3*dy;
	f2=winlines*dy;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	g_flytexcache->sel_tex(flyRender::s_consolepic);
	
	glColor4f(0.7f,0.7f,0.7f,0.85f);
	i=flyRender::s_screensizey+(int)(f1-f2);
	j=flyRender::s_screensizey-(int)f2-4;
	glBegin(GL_QUADS);
		glTexCoord2f(1,1);
		glVertex2i(flyRender::s_screensizex,i);
		glTexCoord2f(0,1);
		glVertex2i(0,i);
		glTexCoord2f(0,0);
		glVertex2i(0,j);
		glTexCoord2f(1,0);
		glVertex2i(flyRender::s_screensizex,j);
	glEnd();
	
	glColor4f(1,1,1,0.85f);
	j=(curr_page-1)*(winlines-1);
	for( i=0;i<winlines-1;i++)
		if (winlines-i-2+j<nlines)
			draw_text(dx,i*dy,buf[winlines-i-2+j]);

	char tmp;
	tmp=cmd_line[cmd_pos];
	cmd_line[cmd_pos]=0;
	f3=(float)draw_text(dx,i*dy+2,cmd_line);
	cmd_line[cmd_pos]=tmp;
	draw_text(f3, i*dy+2, &(cmd_line[cmd_pos]));

	g_flytexcache->sel_tex(-1);
	glColor4f(0,0,0,1);
	i=flyRender::s_screensizey+(int)(f1-f2);
	j=flyRender::s_screensizey-(int)f2-4;

	glPushMatrix();
	glTranslatef(0.5f,-0.5f,0.0f);

	glBegin(GL_LINE_STRIP);
		glVertex2i(flyRender::s_screensizex,i);
		glVertex2i(0,i);
		glVertex2i(0,j);
		glVertex2i(flyRender::s_screensizex,j);
		glVertex2i(flyRender::s_screensizex,i);
	glEnd();

	if (flyRender::s_consolepic==-1)
	{
		i=flyRender::s_screensizey-(int)(f2-dy);
		glBegin(GL_LINES);
			glVertex2i(0,i);
			glVertex2i(flyRender::s_screensizex,i);
		glEnd();
	}

	glPopMatrix();

	unsigned char uc=abs((g_flyengine->cur_time%500)-250);
	glColor4ub(255,255,255,uc);

	f1=f3+1;
	i=winlines-1;
	glBegin(GL_QUADS);
		glVertex2f(f1,flyRender::s_screensizey-i*dy-dy-1);
		glVertex2f(f1+dx/8,flyRender::s_screensizey-i*dy-dy-1);
		glVertex2f(f1+dx/8,flyRender::s_screensizey-i*dy-2);
		glVertex2f(f1,flyRender::s_screensizey-i*dy-2);
	glEnd();
	
	if (flyRender::s_wireframe)
	{
		glPolygonMode(GL_FRONT,GL_LINE);
		glPolygonMode(GL_BACK,GL_LINE);
	}

	glColor4f(1,1,1,1);
}

float flyConsole::draw_text(float x,float y,const char *text) const
{
	if (flyRender::s_fontspic==-1)
		return x;

	int len=strlen(text);
	
	float f=dx/flyRender::s_fontspiccharsize;

	g_flytexcache->sel_tex(flyRender::s_fontspic);

	float xx,yy;
	y=flyRender::s_screensizey-y-dy;

	glBegin(GL_QUADS);
	for( int i=0;i<len;i++ )
		{
		xx=(text[i]%FLY_FONTS_NUM)*FLY_FONTS_FACTOR;
		yy=(text[i]/FLY_FONTS_NUM)*FLY_FONTS_FACTOR;

		glTexCoord2f(xx+FLY_FONTS_FACTOR,yy+FLY_FONTS_FACTOR);
		glVertex2f(x+dx,y);

		glTexCoord2f(xx+FLY_FONTS_FACTOR,yy);
		glVertex2f(x+dx,y+dy);

		glTexCoord2f(xx,yy);
		glVertex2f(x,y+dy);

		glTexCoord2f(xx,yy+FLY_FONTS_FACTOR);
		glVertex2f(x,y);

		x+=flyRender::s_fontswidth[text[i]]*f;
		}
	glEnd();
	
	return x;
}

void flyConsole::key_down(int vk)
{
	int i;
	if(mode)
		switch(vk)
		{
		case 192:
			hide();
			break;
		case VK_PRIOR:
			if(curr_page*(winlines-1)<nlines) curr_page++;
			break;
		case VK_NEXT:
			if(curr_page>1) curr_page--;
			break;
		case VK_DOWN:
			for(i=curr_line-1;i>-1;i--)
				if(buf[i][0]=='>')
				{
					curr_line=i;
					strcpy(cmd_line, &(buf[curr_line][1]));
					cmd_len=strlen(cmd_line);
					cmd_pos=cmd_len;
					break;
				}
			break;
		case VK_UP:
			for(i=curr_line+1;i<nlines;i++)
				if(buf[i][0]=='>')
				{
					curr_line=i;
					strcpy(cmd_line, &(buf[curr_line][1]));
					cmd_len=strlen(cmd_line);
					cmd_pos=cmd_len;
					break;
				}
			break;
		case VK_DELETE:
			if(cmd_line[cmd_pos]!=0)
			{
				strcpy(&cmd_line[cmd_pos], &cmd_line[cmd_pos+1]);
				cmd_len--;
				curr_line=-1;
				curr_page=1;
			}
			break;
		case VK_LEFT:
			if(cmd_pos>0) cmd_pos--;
			break;
		case VK_RIGHT:
			if(cmd_pos<cmd_len) cmd_pos++;
			break;
		case VK_BACK:
			if(cmd_pos>0)
			{
				strcpy(&cmd_line[cmd_pos-1], &cmd_line[cmd_pos]);
				cmd_pos--;
				cmd_len--;
				curr_line=-1;
				curr_page=1;
			}
			break;
		case VK_RETURN:
			curr_line=-1;
			curr_page=1;
			command_exec(cmd_line);
			cmd_line[0]=0;
			cmd_pos=0;
			cmd_len=0;
			break;
		case VK_END:
			cmd_pos=cmd_len;
			break;
		case VK_HOME:
			cmd_pos=0;
			break;
		}
	else 
	if(vk==192)
		show();
}

void flyConsole::key_char(int vk)
{
	if(mode && cmd_len<80 && vk>=32 && vk<=255 && vk!='`')
	{
		memmove(&cmd_line[cmd_pos+1], &cmd_line[cmd_pos], cmd_len-cmd_pos+1);
		cmd_line[cmd_pos++]=vk;
		cmd_len++;
		curr_line=-1;
		curr_page=1;
	}
}

int flyConsole::command_tokens(char *str,char **token) const
{
	int ntoken=0;

	do
	{
	while( *str==' ' )
		*(str++)=0;
	if (*str==0)
		break;
	token[ntoken]=str;

	str=strchr(str,' ');
	if (token[ntoken][0]=='(')
		{
		token[ntoken]++;
		str=strchr(token[ntoken],')');
		if (str)
			*(str++)=0;
		}
	if (token[ntoken][0]=='\"')
		{
		token[ntoken]++;
		str=strchr(token[ntoken],'\"');
		if (str)
			*(str++)=0;
		}

	ntoken++;
	}while(str);

	return ntoken;
}

void flyConsole::command_exec(const char *str) 
{
	static char *token[512],t[512];
	flyString s;
	int ntoken;
	int i,j,k;

	strcpy(t,">");
	strcat(t,str);
	add_string(t);

	ntoken=command_tokens(&t[1],token);

	if (ntoken==0)
		{
		add_string("Invalid command!");
		return;
		}

	if (!stricmp(token[0],"help") || token[0][0]=='?')
		{
		add_string("commands:");
		add_string(" get [ varname | objname.objparam ]");
		add_string(" set [ varname | objname.objparam ] val");
		add_string(" map [ path/level.fly | none]");
		add_string(" save");
		add_string(" listgames [ipaddr]");
		add_string(" connect [ipaddr] [gamenum]");
		add_string(" disconnect");
		add_string(" list [ \"plugins\" | \"classes\" | classname | objname ]");
		add_string(" create classname objname");
		add_string(" destroy objname");
		add_string(" activate objname");
		add_string(" insert plugin.dll");
		add_string(" remove plugin.dll");
		add_string(" playdemo demofile.fdm");
		add_string(" timedemo demofile.fdm");
		add_string(" exec commandline");
		add_string(" quit");
		}
	else
	if (!stricmp(token[0],"playdemo"))
	{
		if (ntoken==1)
			add_string("Missing parameters!");
		else
		{
			g_flyengine->timedemo=0;
			g_flyengine->play_demo(token[1]);
		}
	}
	else
	if (!stricmp(token[0],"timedemo"))
	{
		if (ntoken==1)
			add_string("Missing parameters!");
		else
		{
			g_flyengine->timedemo=1;
			g_flyengine->play_demo(token[1]);
		}
	}
	else
	if (!stricmp(token[0],"map"))
	{
		if (ntoken==1)
			add_string("map: %s",g_flyengine->flyfile[0]==0?"(none)":(const char *)g_flyengine->flyfile);
		else
		{
			g_flyengine->close_fly_file();
			if (!stricmp(token[1],"none"))
				add_string("All map resources released.");
			else 
				g_flyengine->open_fly_file(token[1]);
		}
	}
	else
	if (!stricmp(token[0],"quit"))
		{
		PostMessage(g_flyengine->hwnd,WM_DESTROY,0,0);
		}
	else
	if (!stricmp(token[0],"exec"))
		{
		s=g_flyengine->flysdkpath+token[1];
		WinExec(s,SW_SHOW);
		}
	else
	if (!stricmp(token[0],"disconnect"))
		g_flyengine->close_multiplayer();
	else
	if (!stricmp(token[0],"connect"))
		{
		flyString ipaddr;
		int gamenum=0;
		if (ntoken>1)
		{
			ipaddr=token[1];
			int p=ipaddr.find(':');
			if (p!=-1)
			{
				gamenum=atoi(((const char *)ipaddr)+p+1);
				ipaddr.set_char(p,0);
			}
		}
		if (g_flyengine->join_multiplayer(ipaddr,gamenum))
			add_string("map: %s",g_flyengine->flyfile[0]==0?"(none)":(const char *)g_flyengine->flyfile);
		else
			if (ntoken==1)
				add_string("Error connecting to game!",token[1]);
			else 
				add_string("Error connecting to %s!",token[1]);
		}
	else
	if (!stricmp(token[0],"listgames"))
		{
		flyString ipaddr;
		int gamenum=0;
		if (ntoken>1)
		{
			ipaddr=token[1];
			int p=ipaddr.find(':');
			if (p!=-1)
			{
				gamenum=atoi(((const char *)ipaddr)+p+1);
				ipaddr.set_char(p,0);
			}
		}
		if (g_flyengine->join_multiplayer(ipaddr,-1)==0)
			add_string("Error connecting to %s",token[1]);
		}
	else
	if (!stricmp(token[0],"insert"))
		{
		if (ntoken==1)
			add_string("Missing parameters!");
		else
		{
			i=g_flyengine->dll.add_dll(token[1]);
			if (i==-1)
				add_string("Invalid plugin dll!");
			else g_flyengine->dll.load_classes(i,0);
		}
	}
	else
	if (!stricmp(token[0],"remove"))
		{
		if (ntoken==1)
			add_string("Missing parameters!");
		else
			if (g_flyengine->dll.delete_dll(token[1])==0)
				add_string("Invalid plugin dll!");
		}
	else
	if (!stricmp(token[0],"create"))
		{
		if (ntoken<3)
			add_string("Missing parameters!");
		else
			{
			flyBspObject *obj=g_flyengine->dll.add_class(token[1]);
			if (obj==0)
				add_string("Invalid class name!");
			else
				{
				obj->long_name=token[2];
				obj->name="";
				}
			}
		}
	else
	if (!stricmp(token[0],"destroy"))
		{
		if (ntoken<2)
			add_string("Missing parameters!");
		else
			{
			flyBspObject *o=(flyBspObject *)g_flyengine->active_objs.get_object_longname(token[1]);
			if (o==0)
				add_string("Invalid class name!");
			else 
				o->life=-1;
			}
		}
	else
	if (!stricmp(token[0],"list"))
		{
		if (ntoken==1)
		{
			flyParamDesc pd;
			i=g_flyengine->get_global_param_desc1(0,0);
			j=i+g_flyengine->get_global_param_desc2(0,0);
			for( k=0;k<j;k++ )
				{
				if (k>=i)
					g_flyengine->get_global_param_desc2(k-i,&pd);
				else
					g_flyengine->get_global_param_desc1(k,&pd);
				s=pd.name+"="+pd.get_string();
				add_string(s);
				}
			for( i=0;i<g_flyengine->dll.ndll;i++)
				if (g_flyengine->dll.dll[i]->get_global_param_desc)
				{
					k=g_flyengine->dll.dll[i]->get_global_param_desc(0,0);
					for( j=0;j<k;j++ )
					{
					g_flyengine->dll.dll[i]->get_global_param_desc(j,&pd);
					s=pd.name+"="+pd.get_string();
					add_string(s);
					}
				}
		}
		else
		if (!stricmp(token[1],"plugins"))
			{
			for( i=0;i<g_flyengine->dll.ndll;i++ )
				add_string(g_flyengine->dll.dll[i]->dll_filename);
			}
		else
		if (!stricmp(token[1],"classes"))
			{
			for( i=0;i<g_flyengine->dll.ncd;i++ )
				add_string(g_flyengine->dll.cd[i]->get_name());
			}
		else
		if (strchr(token[1],'.'))
			{
			char *c=strchr(token[1],'.');
			*c=0;
			flyBspObject *o=(flyBspObject *)g_flyengine->stock_objs.get_object_longname(token[1]);
			if (o)
				{
				flyParamDesc pd;
				j=o->get_param_desc(0,0);
				for( k=0;k<j;k++ )
					{
					o->get_param_desc(k,&pd);
					if (!stricmp(c+1,pd.name))
						{
						s=pd.name+"="+pd.get_string();
						add_string(s);
						break;
						}
					}
				if (k==j)
					add_string("Invalid object parameter!");
				}
			else add_string("Invalid object name!");
			}
		else
			{
			for( i=0;i<g_flyengine->dll.ncd;i++ )
				if (!stricmp(token[1],g_flyengine->dll.cd[i]->get_name()))
					break;
			if (i<g_flyengine->dll.ncd)
				{
				j=g_flyengine->dll.cd[i]->get_type();
				flyBspObject *o=(flyBspObject *)g_flyengine->stock_objs.get_next_object(0,j);
				while(o)
					{
					add_string(o->long_name);
					o=(flyBspObject *)g_flyengine->stock_objs.get_next_object(o,j);
					}
				}
			else
				{
				flyBspObject *o=(flyBspObject *)g_flyengine->stock_objs.get_object_longname(token[1]);
				if (o==0)
					add_string("Invalid class/object name!");
				else 
					{
					flyParamDesc pd;
					j=o->get_param_desc(0,0);
					for( k=0;k<j;k++ )
						{
						o->get_param_desc(k,&pd);
						s=pd.name+"="+pd.get_string();
						add_string(s);
						}
					}
				}
			}
		}
	else
	if (!stricmp(token[0],"save"))
		{
		if (g_flyengine->bsp)
			{
			g_flyengine->save_fly_file(g_flyengine->flyfile);
			add_string("Saved!");
			}
		else
			add_string("No current file to save!");
		return;
		}
	else
	if (!stricmp(token[0],"set"))
	{
		if (ntoken<3)
		{
			add_string("Missing parameters!");
			return;
		}

		if (strchr(token[1],'.'))
		{
			char *c=strchr(token[1],'.');
			*c=0;
			i=g_flyengine->set_obj_param(token[1],c+1,token[2]);
			if (i==1)
				add_string("Invalid object name!");
			else if (i==2) 
				add_string("Invalid object parameter!");
			else if (i==3) 
				add_string("Unsuported parameter type!");
			return;
		}

		if (g_flyengine->set_global_param(token[1],token[2])==0)
			add_string("Invalid parameter name!");
	}
	else if (!stricmp(token[0],"activate"))
	{
		flyBspObject *o=(flyBspObject *)g_flyengine->stock_objs.get_object_longname(token[1]);
		if (o)
			g_flyengine->activate(o->clone());
		else add_string("Invalid object name!");
	}
	else if (!stricmp(token[0],"get"))
	{
		if (!strncmp(token[1],"obj(",4))
		{
			char *objname=&token[1][4];
			char *objparam=strchr(token[1],'.');
			char *c=strchr(objname,')');
			if (c==0 || objparam==0)
				{
				add_string("Invalid sintax!");
				return;
				}
			*c=0;
			i=g_flyengine->get_obj_param(objname,objparam+1,s);
			if (i)
			{
			if (i==1)
				add_string("Invalid object name!");
			else if (i==2) 
				add_string("Invalid object parameter!");
			else if (i==3) 
				add_string("Unsuported parameter type!");
			return;
			}
		}
		else
		{
			i=g_flyengine->get_global_param_desc1(0,0);
			j=i+g_flyengine->get_global_param_desc2(0,0);
			flyParamDesc pd;
			for( k=0;k<j;k++ )
				{
				if (k>=i)
					g_flyengine->get_global_param_desc2(k-i,&pd);
				else
					g_flyengine->get_global_param_desc1(k,&pd);
				if (!stricmp(token[1],pd.name))
					break;
				}
			if (k==j)
				{
				add_string("Invalid variable name!");
				return;
				}
			else s=pd.get_string();
		}
		add_string(s);
	}
	else if (!stricmp(token[0],"say"))
	{
		if(ntoken<2)
			add_string("Missing parameters!");
		else
		{
			flyMPMsg msg;
			msg.type=FLY_MP_MSG_CHAT;
			msg.from=g_flydirectx->players[0].dpid;
			strcpy(msg.data,token[1]);
			for( i=2;i<ntoken;i++ )
			{
				strcat(msg.data," ");
				strcat(msg.data,token[i]);
			}
			g_flydirectx->send_message(&msg,9+strlen(msg.data));
		}
			
	}
	else 
		add_string("Invalid command!");
}