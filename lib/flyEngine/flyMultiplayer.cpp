#include "../Fly3D.h"

FLY_ENGINE_API GUID
g_flyGUID = { /* e29ac960-09f8-11d2-9262-000021799871 */
    0xe29ac960,
    0x09f8,
    0x11d2,
    {0x92, 0x62, 0x00, 0x00, 0x21, 0x79, 0x98, 0x71}
  };

void flyEngine::close_multiplayer()
{
	int i;
	for( i=1;i<g_flydirectx->nplayers;i++ )
	{
		flyBspObject *obj=(flyBspObject *)g_flydirectx->players[i].data;
		obj->life=-1;
	}

	delete demo_file_in;
	demo_file_in=0;

	g_flydirectx->free_multiplayer();
}

int flyEngine::join_multiplayer(const char *addr,int num)
{
	close_fly_file();
	close_multiplayer();

	flyMPGames *g=0;
	g_flydirectx->init_multiplayer(addr);
	g=g_flydirectx->enum_games(&g_flyGUID);
	if (g==0)
	{
		g_flydirectx->free_multiplayer();
		return 0;
	}

	if (num==-1)
	{
		int i;
		for( i=0;i<g->num;i++ )
			if (g->num_players[i]==0)
				console_printf("%i: %s, no players",i,g->name[i]);
			else
			if (g->num_players[i]==1)
				console_printf("%i: %s, 1 player",i,g->name[i]);
			else
				console_printf("%i: %s, %i player",i,g->name[i],g->num_players[i]);
		return 1;
	}
	if (g->num>0)
	{
		unsigned color=D3DRGB(g_flyengine->playercolor.x,
			g_flyengine->playercolor.y,g_flyengine->playercolor.z);
		g_flydirectx->join_game(&g->guid[num%g->num],playername,color);
		level_start_time=cur_time+1;
		int maxtime=5000;
		while(maxtime>0)
		{
			maxtime-=500;
			Sleep(500);
			check_multiplayer();
			if (flyfile[0]!=0) 
				break;
		}

		if (flyfile[0]!=0)
			return 1;
	}
	close_multiplayer();
	return 0;
}

void flyEngine::check_multiplayer()
{
	if (demo_file_in)
	{
		static flyMPMsg msg;
		int i,size,time,time0=-1,from;
		static int last_time=-1;
		if (demo_file_in->pos==0)
			last_time=-1;
		while(1)
		{
			i=demo_file_in->read(&size,sizeof(int));
			i+=demo_file_in->read(&time,sizeof(int));
			if (i!=8)
				break;
			if (time0==-1)
				time0=time;
			if ((timedemo==1 && time!=time0) ||
				(timedemo==0 && time>cur_time-level_start_time))
				{
				demo_file_in->seek(-8);
				timedemoframes++;
				break;
				}
			demo_file_in->read(&msg.type,size);
			for( from=0;from<g_flydirectx->nplayers;from++ )
				if (g_flydirectx->players[from].dpid==msg.from)
					break;
			if (from==g_flydirectx->nplayers)
				from=-1;
			process_multiplayer(from,size,&msg);
		}
		if (timedemo==1 && last_time!=-1)
			force_dt=time0-last_time;
		else
			force_dt=0;
		last_time=time0;

		if(i!=8 || g_flydirectx->mouse_down!=0)
		{
			if (i!=8)
				console_printf("demo playback finished.");
			else
				console_printf("demo playback interrupted.");
			if (timedemo)
				console_printf("timedemo: %i frames, %.2f sec, %.2f fps",
					timedemoframes,(cur_time-level_start_time)/1000.0f,
					(float)timedemoframes*1000/(cur_time-level_start_time));
			close_multiplayer();
			console_command="map menu.fly";
		}
	}
	else
	{
		int i,num,from,time;
		flyMPMsg *msg;
		DWORD size;

		num=g_flydirectx->get_num_messages();
		for( i=0;i<num;i++ )
		{
			size=500;
			msg=g_flydirectx->get_message(&size);
			if (msg)
			{
				for( from=0;from<g_flydirectx->nplayers;from++ )
					if (g_flydirectx->players[from].dpid==msg->from)
						break;
				if (from==g_flydirectx->nplayers)
					from=-1;
				
				if (demo_file_out)
				{
					fwrite(&size,sizeof(int),1,demo_file_out);
					time=cur_time-level_start_time;
					fwrite(&time,sizeof(int),1,demo_file_out);
					fwrite(&msg->type,size,1,demo_file_out);
				}

				process_multiplayer(from,size,msg);
			}
		}
	}
}

void flyEngine::process_multiplayer(int from,int size,const flyMPMsg *msg)
{
	int i,j;
	switch(msg->type)
	{
	case DPSYS_CREATEPLAYERORGROUP:
		{
			DPMSG_CREATEPLAYERORGROUP *info=(DPMSG_CREATEPLAYERORGROUP *)&msg->type;
			flyMPPlayerData *data=(flyMPPlayerData *)info->lpData;

			flyMPMsg msgflyfile;
			strcpy(msgflyfile.data,flyfile);
			msgflyfile.type=FLY_MP_MSG_FLYFILE;
			msgflyfile.from=info->dpId;
			g_flydirectx->send_message(&msgflyfile,strlen(flyfilename)+9,info->dpId);
			
			flyMPPlayerData pd;
			pd.type=FLY_MP_MSG_JOIN;
			pd.dpid=info->dpId;
			pd.color=data->color;
			strcpy(pd.name,info->dpnName.lpszShortNameA);
			dll.send_message(FLY_MESSAGE_MPMESSAGE,-1,&pd);

			g_flydirectx->players[g_flydirectx->nplayers-1].type=FLY_MP_MSG_JOIN;
			for( i=1;i<g_flydirectx->nplayers-1;i++ )
				{
				g_flydirectx->players[i].type=FLY_MP_MSG_JOIN;
				g_flydirectx->send_message((flyMPMsg *)&g_flydirectx->players[i],sizeof(flyMPPlayerData),info->dpId);
				g_flydirectx->send_message((flyMPMsg *)&g_flydirectx->players[g_flydirectx->nplayers-1],sizeof(flyMPPlayerData),g_flydirectx->players[i].dpid);
				}
		}
		break;
	case DPSYS_DESTROYPLAYERORGROUP:
		{
			DPMSG_DESTROYPLAYERORGROUP *info=(DPMSG_DESTROYPLAYERORGROUP *)&msg->type;
			for( j=0;j<g_flydirectx->nplayers;j++ )
				if (g_flydirectx->players[j].dpid==info->dpId)
					break;
			if (j<g_flydirectx->nplayers)
				{
				g_flydirectx->players[j].type=FLY_MP_MSG_QUIT;
				g_flydirectx->send_message((flyMPMsg *)&g_flydirectx->players[j],sizeof(flyMPPlayerData));
				dll.send_message(FLY_MESSAGE_MPMESSAGE,j,&g_flydirectx->players[j]);
				}
		}
		break;
	case DPSYS_SESSIONLOST:
		close_multiplayer();
		return;
	case FLY_MP_MSG_CHAT:
		if (from>=0 && from<g_flydirectx->nplayers)
			console_printf("%s: %s",g_flydirectx->players[from].name,msg->data);
		else if (from==-1)
			console_printf("Server: %s",msg->data);
		if (g_flydirectx->mpmode==FLY_MP_SERVER)
			g_flydirectx->send_message(msg,size);
		break;
	case FLY_MP_MSG_FLYFILE:
		open_fly_file(msg->data);
		if (flyfile[0]!=0)
		{
			g_flydirectx->players[0].data=player;
			g_flydirectx->players[0].dpid=msg->from;
		}
		break;
	default:
		dll.send_message(FLY_MESSAGE_MPMESSAGE,from,(void *)msg);
		if (g_flydirectx->mpmode==FLY_MP_SERVER)
			g_flydirectx->send_message(msg,size);
	}
}

void flyEngine::record_demo(const char *file)
{ 
	if (demo_file_out) 
		fclose(demo_file_out); 
	demo_file_out=0; 
	if (file)
		demo_file_out=fopen(flyString(flysdkdatapath+file),"wb"); 
}

void flyEngine::play_demo(const char *file)
{
	close_fly_file();
	close_multiplayer();
	record_demo(0);

	timedemoframes=0;
	
	demo_file_in=new flyFile;
	demo_file_in->open(flyString(flysdkdatapath+file));
	if (demo_file_in->buf==0)
	{
		console_printf("demo file not found!");
		delete demo_file_in;
		demo_file_in=0;
		return;
	}

	unsigned color=D3DRGB(playercolor.x,playercolor.y,playercolor.z);
	strcpy(g_flydirectx->players[0].name,playername);
	g_flydirectx->players[0].color=color;
	g_flydirectx->players[0].data=0;
	g_flydirectx->nplayers=1;

	level_start_time=cur_time+1;
	g_flydirectx->mpmode=1;
	check_multiplayer();

	if (flyfile[0]==0)
	{
		close_multiplayer();
		console_printf("demo map not found!");
		console_command="map menu.fly";
	}
	else
		console_printf("playing demo file...");
}
