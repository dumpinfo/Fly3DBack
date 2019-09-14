#include "../flyDirectX.h"

float flyDirectx::s_mousespeed=0.1f;
int flyDirectx::s_invertmousex=0;
int flyDirectx::s_invertmousey=1;

#ifndef FLY_WIN_NT4_SUPORT
float flyDirectx::s_joyspeed=2.0f;
int flyDirectx::s_invertjoyx=0;
int flyDirectx::s_invertjoyy=1;
#endif

FLY_DIRECTX_API flyDirectx *g_flydirectx=0;

FLY_DIRECTX_API void fly_init_directx(HWND hwnd,HINSTANCE hinst)
{
	fly_free_directx();
	g_flydirectx=new flyDirectx(hwnd,hinst);
}

FLY_DIRECTX_API void fly_free_directx()
{
	delete g_flydirectx;
	g_flydirectx=0;
}

void flyDirectx::send_message(const flyMPMsg *data,int len,DWORD dpid)
{
	if (lpDP)
		if (data->type&FLY_MP_GUARANTEED)
			if (dpid==0)
				lpDP->Send(players[0].dpid,DPID_ALLPLAYERS,DPSEND_GUARANTEED,(void *)data,len);
			else lpDP->Send(players[0].dpid,dpid,DPSEND_GUARANTEED,(void *)data,len);
		else
			if (dpid==0)
				lpDP->Send(players[0].dpid,DPID_ALLPLAYERS,0,(void *)data,len);
			else lpDP->Send(players[0].dpid,dpid,0,(void *)data,len);
}

BOOL FAR PASCAL EnumAddressCallback(
	REFGUID guidDataType,DWORD dwDataSize,
	LPCVOID lpData,LPVOID lpContext)
{
	if (guidDataType==DPAID_INet)
	{
		if (((char *)lpContext)[0]!=0)
			strcat((char *)lpContext," , ");
		strcat((char *)lpContext,(char *)lpData);
	}
	return 1;
}

char *flyDirectx::get_player_address(DWORD dpid)
{
	static char buf[1024],str[256];
	str[0]=0;
	if (lpDP)
	{
		DWORD size=1024;
		lpDP->GetPlayerAddress(dpid,buf,&size);
		lpDPL->EnumAddress(EnumAddressCallback,buf,size,str);
	}
	return str;
}

int flyDirectx::get_num_messages()
{
	DWORD count=0;
	if (lpDP)
		lpDP->GetMessageCount(players[0].dpid,&count);	
	return count;
}

flyMPMsg *flyDirectx::get_message(DWORD *size)
{
	if (lpDP==0)
		return 0;
	static flyMPMsg msg;
	DWORD idfrom;
	DWORD idto=players[0].dpid;
	if (DP_OK==lpDP->Receive(&idfrom,&idto,DPRECEIVE_TOPLAYER,&msg.type,size))
		return &msg;
	return 0;
}

int flyDirectx::init_multiplayer(const char *netaddress)
{
	DPCOMPOUNDADDRESSELEMENT addressElements[3];
	DWORD dwElementCount=0;
	char address[512];
	DWORD size=512;

	free_multiplayer();
	CoCreateInstance(CLSID_DirectPlay, NULL, CLSCTX_INPROC_SERVER,
          IID_IDirectPlay3A,(LPVOID*)&lpDP);
	if (lpDP)
		{
		CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER,
			IID_IDirectPlayLobby2A, (LPVOID *) &lpDPL);

		addressElements[dwElementCount].guidDataType = DPAID_ServiceProvider;
		addressElements[dwElementCount].dwDataSize = sizeof(GUID);
		addressElements[dwElementCount].lpData = (LPVOID) &DPSPGUID_TCPIP;
		dwElementCount++;
		
		if (netaddress)
			{
			addressElements[dwElementCount].guidDataType = DPAID_INet;
			addressElements[dwElementCount].dwDataSize = strlen(netaddress)+1;
			addressElements[dwElementCount].lpData = (void *)netaddress;
			dwElementCount++;
			}

		lpDPL->CreateCompoundAddress(
			addressElements, dwElementCount,
			address, &size);

		if (DP_OK==lpDP->InitializeConnection(address,0))
			return 1;
		}
	free_multiplayer();
	return 0;
}

void flyDirectx::free_multiplayer()
{
	if (lpDPL)
		lpDPL->Release();
	if (lpDP)
		lpDP->Release();
	delete lpSD;
	lpSD=0;
	lpDP=0;
	lpDPL=0;
	nplayers=0;
	mpmode=FLY_MP_NOMP;
}

BOOL FAR PASCAL EnumPlayersCallback(
	DPID dpId,DWORD dwPlayerType,
	LPCDPNAME lpName,DWORD dwFlags,	LPVOID lpContext)
{
	flyDirectx *dx=(flyDirectx *)lpContext;

	if (dpId!=DPID_SERVERPLAYER && dpId!=dx->players[0].dpid)
	{
		strcpy(dx->players[dx->nplayers].name,lpName->lpszShortNameA);
		dx->players[dx->nplayers].dpid=dpId;
		dx->nplayers++;
	}
	return 1;
}

int flyDirectx::join_game(LPGUID game_guid,const char *player_name,unsigned color)
{
	if (lpDP==0) return 0;

	mpmode=0;

	lpSD=new DPSESSIONDESC2;
	memset(lpSD,0,sizeof(DPSESSIONDESC2));
	
	lpSD->guidInstance=guidInstance=*game_guid;
	lpSD->dwSize=sizeof(DPSESSIONDESC2);
	if (DP_OK!=lpDP->Open(lpSD,DPOPEN_JOIN))
		return 0;

	nplayers=1;
	strcpy(players[0].name,player_name);
	players[0].color=color;
	players[0].data=0;

	DPNAME pn;
	pn.dwFlags=0;
	pn.dwSize=sizeof(DPNAME);
	pn.lpszShortNameA=players[0].name;
	pn.lpszLongNameA=players[0].name;
	lpDP->CreatePlayer(&players[0].dpid,&pn,0,(void *)&players[0],sizeof(flyMPPlayerData),0);

	lpDP->EnumPlayers(0,EnumPlayersCallback,this,DPENUMPLAYERS_ALL);

	mpmode=1;

	return players[0].dpid;
}

int flyDirectx::create_game(LPGUID app_guid,const char *game_name)
{
	if (lpDP==0) return 0;

	lpSD=new DPSESSIONDESC2;
	memset(lpSD,0,sizeof(DPSESSIONDESC2));
	
	CoCreateGuid(&guidInstance);
	lpSD->guidInstance=guidInstance;

	lpSD->dwSize=sizeof(DPSESSIONDESC2);
	lpSD->dwFlags=
		DPSESSION_CLIENTSERVER|
		DPSESSION_KEEPALIVE|
		DPSESSION_NODATAMESSAGES;
	lpSD->guidApplication=*app_guid;
	lpSD->dwMaxPlayers=FLY_MP_MAXPLAYERS;
	lpSD->dwCurrentPlayers=0;
	lpSD->lpszSessionNameA=(char *)game_name;

	if (DP_OK!=lpDP->Open(lpSD,DPOPEN_CREATE))
		return 0;

	DPNAME pn;
	pn.dwFlags=0;
	pn.dwSize=sizeof(DPNAME);
	nplayers=1;
	players[0].data=0;
	strcpy(players[0].name,"server");
	pn.lpszShortNameA="server";
	pn.lpszLongNameA="server";
	lpDP->CreatePlayer(&players[0].dpid,&pn,0,0,0,DPPLAYER_SERVERPLAYER);
	mpmode=2;

	return 1;
}

BOOL FAR PASCAL EnumSessionsCallback(
	LPCDPSESSIONDESC2 lpThisSD,	LPDWORD lpdwTimeOut,
	DWORD dwFlags,LPVOID lpContext)
{
	if (lpThisSD==0) return 0;
	flyMPGames *g=(flyMPGames *)lpContext;
	strcpy(g->name[g->num],lpThisSD->lpszSessionNameA);
	g->guid[g->num]=lpThisSD->guidInstance;
	g->num_players[g->num]=lpThisSD->dwCurrentPlayers-1;
	g->num++;
	return 1;
}

flyMPGames *flyDirectx::enum_games(LPGUID app_guid)
{
	if (lpDP==0) return 0;

	static flyMPGames g;
	g.num=0;

	DPSESSIONDESC2 sd;
	memset(&sd,0,sizeof(DPSESSIONDESC2));
	sd.dwSize=sizeof(DPSESSIONDESC2);
	sd.guidApplication=*app_guid;
	lpDP->EnumSessions(&sd,FLY_MP_BROWSETIME,EnumSessionsCallback,&g,DPENUMSESSIONS_ALL);

	return &g;
}

void flyDirectx::Init()
{
    HRESULT hr;
	CoInitialize(0);
    hr = DirectInputCreate(hInst, DIRECTINPUT_VERSION, &lpDI, NULL);
	if (lpDI!=NULL)
	{
    hr = lpDI->CreateDevice(GUID_SysKeyboard, &lpKeyboard, NULL);
	if (lpKeyboard)
		{
		hr = lpKeyboard->SetDataFormat(&c_dfDIKeyboard);
		hr = lpKeyboard->SetCooperativeLevel(hWnd, DSSCL_NORMAL);//DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
		}

    hr = lpDI->CreateDevice(GUID_SysMouse, &lpMouse, NULL);
	if (lpMouse)
		{
		hr = lpMouse->SetDataFormat(&c_dfDIMouse);
		hr = lpMouse->SetCooperativeLevel(hWnd, DSSCL_NORMAL);//DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
		}
	}
	if (lpKeyboard) lpKeyboard->Acquire();
	if (lpMouse) lpMouse->Acquire();

#ifndef FLY_WIN_NT4_SUPORT
	if (lpDI!=NULL)
	{
    hr = lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, joystick_enum, this, DIEDFL_ATTACHEDONLY);
	if (lpJoystick)
	    {
		// set data format
		hr = lpJoystick->SetDataFormat(&c_dfDIJoystick);
		// set cooperation level
		hr = lpJoystick->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
		// get capabilites
		joy_caps.dwSize = sizeof(DIDEVCAPS);
		hr = lpJoystick->GetCapabilities(&joy_caps);
		// axes
		joy_axes=0;
		hr = lpJoystick->EnumObjects(joystick_enum_axes, this, DIDFT_AXIS);
	    }
	}
	if (lpJoystick) lpJoystick->Acquire();
#endif

	hr = DirectSoundCreate(NULL,&lpDSound,NULL);
	if (lpDSound!=NULL)
	{
		lpDSound->SetCooperativeLevel(hWnd,DSSCL_PRIORITY);
		
		DSBUFFERDESC desc;
		memset(&desc,0,sizeof(DSBUFFERDESC));
		desc.dwSize=sizeof(DSBUFFERDESC);
		desc.dwFlags=DSBCAPS_CTRL3D|DSBCAPS_CTRLVOLUME|DSBCAPS_PRIMARYBUFFER;
		lpDSound->CreateSoundBuffer(&desc,&lpDS3dPrimBuf,0);

		WAVEFORMATEX format;
		format.wFormatTag=WAVE_FORMAT_PCM;
		format.nChannels=2;
		format.nSamplesPerSec=22050;
		format.wBitsPerSample=16;
		format.nBlockAlign=format.nChannels*format.wBitsPerSample/8;
		format.nAvgBytesPerSec=format.nSamplesPerSec*format.nBlockAlign;
		format.cbSize=0;
		lpDS3dPrimBuf->SetFormat(&format);

		lpDS3dPrimBuf->QueryInterface(IID_IDirectSound3DListener, (void **)(&lpDS3dListener));
	}
	zero_input();
}

void flyDirectx::Release()
{
	free_multiplayer();
    if (lpKeyboard) 
		{
        lpKeyboard->Unacquire();
        lpKeyboard->Release();
        lpKeyboard = NULL;
		}
    if (lpMouse) 
		{
        lpMouse->Unacquire();
        lpMouse->Release();
        lpMouse = NULL;
		}
#ifndef FLY_WIN_NT4_SUPORT
	if (lpJoystick)
		{
        lpJoystick->Unacquire();
        lpJoystick->Release();
        lpJoystick=0;
        lpJoystickPoll=0;
		}
#endif
    if (lpDI) 
		{
        lpDI->Release();
        lpDI = NULL;
		}
	if (lpDSound)
		{
		lpDS3dListener->Release();
		lpDS3dPrimBuf->Release();
        lpDSound->Release();
        lpDSound = NULL;
		}
	CoUninitialize();
}

int flyDirectx::load_wav_file(LONG cchBuffer, HPSTR pchBuffer, LPDIRECTSOUNDBUFFER *buf,LPDIRECTSOUND3DBUFFER *buf3d)
{
	if (lpDSound==0)
		return 0;

	UINT Size;
	DWORD Samples;
	WAVEFORMATEX *Info;
	BYTE *Data;
	DSBUFFERDESC desc;

	if (WaveLoadFileB(cchBuffer,pchBuffer,&Size,&Samples,&Info,&Data))
		return 0;
	desc.dwSize=sizeof(DSBUFFERDESC);
	desc.dwFlags=DSBCAPS_STATIC|DSBCAPS_CTRLVOLUME;
	if (buf3d)
		desc.dwFlags|=DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE;
	desc.dwBufferBytes=Size;
    desc.lpwfxFormat=Info;
	desc.dwReserved=0;

	lpDSound->CreateSoundBuffer(&desc,buf,0);
	if (*buf)
		{
		LPVOID blk1,blk2;
		DWORD size1,size2;

		if (buf3d)
			(*buf)->QueryInterface(IID_IDirectSound3DBuffer,(void **)(buf3d));

		(*buf)->Lock(0,Size,&blk1,&size1,&blk2,&size2,0);
		CopyMemory(blk1,Data,size1);
		if (Size>size1)
			CopyMemory(blk2,Data+size1,size2);
		(*buf)->Unlock(blk1,size1,blk2,size2);
		}

	Size=Size*1000/Info->nAvgBytesPerSec;
	if (Info)
		GlobalFree(Info);
	if (Data)
		GlobalFree(Data);
	Info=0;
	Data=0;
	return Size;
}

LPDIRECTSOUNDBUFFER flyDirectx::clone_sound(LPDIRECTSOUNDBUFFER buf)
{
	if (lpDSound==0) return 0;
	static LPDIRECTSOUNDBUFFER dup_buf;
	lpDSound->DuplicateSoundBuffer(buf,&dup_buf);
	return dup_buf;
}


void flyDirectx::zero_input()
{
	memset(keys,0,256);
	mouse_pos[0]=0;
	mouse_pos[1]=0;
	mouse_delta[0]=0.0f;
	mouse_delta[1]=0.0f;
	mouse_smooth[0]=0.0f;
	mouse_smooth[1]=0.0f;
	mouse_down=0;
	mouse_click=0;
#ifndef FLY_WIN_NT4_SUPORT
	joy_down=0;
	joy_pos[0]=0;
	joy_pos[1]=0;
#endif
}

void flyDirectx::get_input()
{
	if (lpDI==0) return;

	HRESULT hr;
	static DIMOUSESTATE dims;

	if (lpKeyboard) 
	{
		hr = lpKeyboard->GetDeviceState(sizeof(keys), keys);
		if (hr == DIERR_INPUTLOST) 
			{
			hr = lpKeyboard->Acquire();
			if (SUCCEEDED(hr)) 
				hr = lpKeyboard->GetDeviceState(sizeof(keys), keys);
			}
	}
	if (lpMouse) 
	{
		hr=lpMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
		if (hr == DIERR_INPUTLOST) 
			{
			hr = lpMouse->Acquire();
			if (SUCCEEDED(hr)) 
				hr = lpMouse->GetDeviceState(sizeof(DIMOUSESTATE), &dims);
			}
	}
#ifndef FLY_WIN_NT4_SUPORT
	static DIJOYSTATE joy_state;
    if (lpJoystickPoll)
    {
		hr = lpJoystickPoll->Poll();

        hr = lpJoystickPoll->GetDeviceState(sizeof(DIJOYSTATE), &joy_state);
        if (hr == DIERR_INPUTLOST)
        {
            hr = lpJoystickPoll->Acquire();
            
			if (SUCCEEDED(hr))
                hr = lpJoystickPoll->GetDeviceState(sizeof(DIJOYSTATE), &joy_state);
		}
    }
	joy_pos[0]=s_joyspeed*(s_invertjoyx?-joy_state.lX:joy_state.lX)/(joy_max_range-joy_min_range);
	joy_pos[1]=s_joyspeed*(s_invertjoyy?-joy_state.lY:joy_state.lY)/(joy_max_range-joy_min_range);
	joy_down = 0;
	if(joy_state.rgbButtons[0])
		joy_down+=FLY_JOY_BUTTON_1;
	if(joy_state.rgbButtons[1])
		joy_down+=FLY_JOY_BUTTON_2;
	if(joy_state.rgbButtons[2])
		joy_down+=FLY_JOY_BUTTON_3;
	if(joy_state.rgbButtons[3])
		joy_down+=FLY_JOY_BUTTON_4;
	if(joy_state.rgbButtons[4])
		joy_down+=FLY_JOY_BUTTON_5;
	if(joy_state.rgbButtons[5])
		joy_down+=FLY_JOY_BUTTON_6;
	if(joy_state.rgbButtons[6])
		joy_down+=FLY_JOY_BUTTON_7;
	if(joy_state.rgbButtons[7])
		joy_down+=FLY_JOY_BUTTON_8;
	if(joy_state.rgbButtons[8])
		joy_down+=FLY_JOY_BUTTON_9;
	if(joy_state.rgbButtons[9])
		joy_down+=FLY_JOY_BUTTON_10;
#endif 

	POINT p;
	char mouse_down_ant=mouse_down;

	GetCursorPos(&p);
	ScreenToClient(hWnd, &p);

	mouse_pos[0]=p.x;
	mouse_pos[1]=p.y;

	mouse_delta[0]=s_mousespeed*(s_invertmousex?-dims.lX:dims.lX);
	mouse_delta[1]=s_mousespeed*(s_invertmousey?-dims.lY:dims.lY);

	mouse_down=0;
	mouse_click=0;

	if(dims.rgbButtons[0])
		mouse_down+=FLY_MOUSE_L;

	if(dims.rgbButtons[1])
		mouse_down+=FLY_MOUSE_R;

	if(dims.rgbButtons[2])
		mouse_down+=FLY_MOUSE_M;

	if(!(mouse_down_ant&FLY_MOUSE_L) && mouse_down&FLY_MOUSE_L)
		mouse_click+=FLY_MOUSE_L;

	if(!(mouse_down_ant&FLY_MOUSE_R) && mouse_down&FLY_MOUSE_R)
		mouse_click+=FLY_MOUSE_R;

	if(!(mouse_down_ant&FLY_MOUSE_M) && mouse_down&FLY_MOUSE_M)
		mouse_click+=FLY_MOUSE_M;

	// mouse smooth
	static float lastmouse[2][2]={ { 0,0 },{ 0,0 } };
	static int lm=0;
	
	mouse_smooth[0]=(mouse_delta[0]+lastmouse[0][0]+lastmouse[1][0])/3.0f;
	mouse_smooth[1]=(mouse_delta[1]+lastmouse[0][1]+lastmouse[1][1])/3.0f;

	lastmouse[lm][0]=mouse_delta[0];
	lastmouse[lm][1]=mouse_delta[1];

	lm=!lm;
}

void flyDirectx::set_listener(const float *pos,const float *vel,const float *Y,const float *Z)
{
	if (lpDS3dListener)
	{
		if (pos)
		lpDS3dListener->SetPosition(
			pos[0],pos[1],pos[2],
			DS3D_DEFERRED);
		if (vel)
		lpDS3dListener->SetVelocity(
			vel[0],vel[1],vel[2],
			DS3D_DEFERRED);
		if (Y && Z)
		lpDS3dListener->SetOrientation(
			Z[0],Z[1],Z[2],
			Y[0],Y[1],Y[2],
			DS3D_DEFERRED);
		lpDS3dListener->CommitDeferredSettings();
	}
}

int flyDirectx::add_player(const char *name,DWORD dpid,void *data,unsigned color)
{
	if (nplayers<FLY_MP_MAXPLAYERS)
	{
		strcpy(players[nplayers].name,name);
		players[nplayers].dpid=dpid;
		players[nplayers].color=color;
		players[nplayers].data=data;
		return nplayers++;
	}
	else return -1;
}

void *flyDirectx::remove_player(int i)
{
	if (i<nplayers)
	{
		void *data=players[i].data;
		memcpy(&players[i],&players[i+1],sizeof(flyMPPlayerData)*(nplayers-i-1));
		nplayers--;
		return data;
	}
	else return 0;
}

void flyDirectx::set_master_volume(int volume)
{
	if (lpDS3dPrimBuf)
		lpDS3dPrimBuf->SetVolume(volume);
}

#ifndef FLY_WIN_NT4_SUPORT
BOOL CALLBACK joystick_enum(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
    HRESULT hr;

	flyDirectx *dx=(flyDirectx *)pContext;

    // obtain an interface to the enumerated joystick.
    hr = dx->lpDI->CreateDevice(pdidInstance->guidInstance, &dx->lpJoystick, NULL);
    if(FAILED(hr)) return DIENUM_CONTINUE;
    
	// get device2 for it so we can poll the joystick
	if (dx->lpJoystick)
	{
		hr = dx->lpJoystick->QueryInterface( 
				IID_IDirectInputDevice2,                     
				(LPVOID *)&dx->lpJoystickPoll);         
		if(FAILED(hr)) return DIENUM_CONTINUE;
	}

    return DIENUM_STOP;
}
BOOL CALLBACK joystick_enum_axes(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
	HRESULT hr;
    // Set the range for the axis

	flyDirectx *dx=(flyDirectx *)pContext;

    DIPROPRANGE diprg; 
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYOFFSET; 
    diprg.diph.dwObj        = pdidoi->dwOfs; // Specify the enumerated axis
    diprg.lMin              = dx->joy_min_range;
    diprg.lMax              = dx->joy_max_range;
    
    hr = dx->lpJoystick->SetProperty(DIPROP_RANGE, &diprg.diph);
	if (FAILED(hr)) return DIENUM_STOP;

    // Set the dead zone for the axis
	DIPROPDWORD dipdw;     
	dipdw.diph.dwSize       = sizeof(DIPROPDWORD);    
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);    
	dipdw.diph.dwObj        = pdidoi->dwOfs;    
	dipdw.diph.dwHow        = DIPH_BYOFFSET;    
	dipdw.dwData            = dx->joy_dead_zone;     

    hr = dx->lpJoystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
	if (FAILED(hr)) return DIENUM_STOP;

    // Set the UI to reflect what axes the joystick supports
    switch( pdidoi->dwOfs )
    {
        case DIJOFS_X:
			dx->joy_axes|=FLY_AXIS_X;
            break;
        case DIJOFS_Y:
			dx->joy_axes|=FLY_AXIS_Y;
            break;
        case DIJOFS_Z:
			dx->joy_axes|=FLY_AXIS_Z;
            break;
        case DIJOFS_RX:
			dx->joy_axes|=FLY_AXIS_X_ROT;
            break;
        case DIJOFS_RY:
			dx->joy_axes|=FLY_AXIS_Y_ROT;
            break;
        case DIJOFS_RZ:
			dx->joy_axes|=FLY_AXIS_Z_ROT;
            break;
        case DIJOFS_SLIDER(0):
			dx->joy_axes|=FLY_AXIS_SLIDER0;
            break;
        case DIJOFS_SLIDER(1):
			dx->joy_axes|=FLY_AXIS_SLIDER1;
            break;
    }

    return DIENUM_CONTINUE;
}
#endif
