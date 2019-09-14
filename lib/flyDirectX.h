/*! \file */

#ifdef FLY_DIRECTX_H
#else
#define FLY_DIRECTX_H

#ifdef FLY_DIRECTX_EXPORTS
#define FLY_DIRECTX_API __declspec(dllexport)
#else
#define FLY_DIRECTX_API __declspec(dllimport)
#endif

//#define FLY_WIN_NT4_SUPORT

#ifdef FLY_WIN_NT4_SUPORT
	//! DirectInput version 3 used
	#define DIRECTINPUT_VERSION 0x0300
#else
	//! DirectInput version 5 used
	#define DIRECTINPUT_VERSION 0x0500
#endif

#include <windows.h>
#include <stdio.h>
#include "dinput.h"
#include "dsound.h"
#include "dplay.h"
#include "dplay.h"
#include "dplobby.h"

int WaveLoadFile(char *, UINT *, DWORD *, WAVEFORMATEX **, BYTE **);
int WaveLoadFileB(LONG, HPSTR, UINT *, DWORD *, WAVEFORMATEX **, BYTE **);

#ifndef FLY_WIN_NT4_SUPORT
BOOL CALLBACK joystick_enum(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
BOOL CALLBACK joystick_enum_axes(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext);
#endif

//! maximum browse time
#define FLY_MP_BROWSETIME 2000
//! player data structure size
#define FLY_MP_MAXPLAYERS 16
//! maximum number of services
#define FLY_MP_MAXSERVICES 16
//! maximum number of simultaneous games
#define FLY_MP_MAXGAMES 16

//! flag for guaranteed message delivery
#define FLY_MP_GUARANTEED 0x80000000

//! 'join' multiplayer message
#define FLY_MP_MSG_JOIN 9171|FLY_MP_GUARANTEED
//! 'quit' multiplayer message
#define FLY_MP_MSG_QUIT 9172|FLY_MP_GUARANTEED
//! 'open .fly file' multiplayer message
#define FLY_MP_MSG_FLYFILE 9173|FLY_MP_GUARANTEED
//! multiplayer chat message
#define FLY_MP_MSG_CHAT 9174|FLY_MP_GUARANTEED

//! multiplayer mode: no multiplayer
#define FLY_MP_NOMP 0
//! multiplayer mode: client
#define FLY_MP_CLIENT 1
//! multiplayer mode: server
#define FLY_MP_SERVER 2

//! mouse left button
#define	FLY_MOUSE_L 1
//! mouse right button
#define FLY_MOUSE_R 2
//! mouse middle button
#define FLY_MOUSE_M 4

//! joystick X axis
#define	FLY_AXIS_X			1
//! joystick Y axis
#define	FLY_AXIS_Y			2
//! joystick Z axis
#define	FLY_AXIS_Z			4
//! joystick X axis rotation
#define	FLY_AXIS_X_ROT		8
//! joystick Y axis rotation
#define	FLY_AXIS_Y_ROT		16
//! joystick Z axis rotation
#define	FLY_AXIS_Z_ROT		32
//! joystick first slider
#define	FLY_AXIS_SLIDER0	64
//! joystick second slider
#define	FLY_AXIS_SLIDER1	128

//! joystick button number 1
#define	FLY_JOY_BUTTON_1	1
//! joystick button number 2
#define	FLY_JOY_BUTTON_2	2
//! joystick button number 3
#define	FLY_JOY_BUTTON_3	4
//! joystick button number 4
#define	FLY_JOY_BUTTON_4	8
//! joystick button number 5
#define	FLY_JOY_BUTTON_5	16
//! joystick button number 6
#define	FLY_JOY_BUTTON_6	32
//! joystick button number 7
#define	FLY_JOY_BUTTON_7	64
//! joystick button number 8
#define	FLY_JOY_BUTTON_8	128
//! joystick button number 9
#define	FLY_JOY_BUTTON_9	256
//! joystick button number 10
#define	FLY_JOY_BUTTON_10	512

//! Multiplayer message
/*!
	This class implements a minimized data structure for exchanging 
	info in a multiplayer game. Among its members are a message type
	identifier, a sender identifier and the data buffer itself.
*/
class FLY_DIRECTX_API flyMPMsg 
{
public:
	DWORD type;		//!< message type
	DWORD from;		//!< message sender player index
	char data[500];	//!< message data of 500 bytes maximum
};

//! Multiplayer games database
/*!
	This class implements a databse that holds all information concerning 
	the currently online multiplayer game sessions, with information about the total
	number of sessions and the name and number of players in each session.
*/
class FLY_DIRECTX_API flyMPGames
{
public:
	int num;							//!< number of games
	char name[FLY_MP_MAXGAMES][128];	//!< games' names
	int num_players[FLY_MP_MAXGAMES];	//!< number of players in each game
	GUID guid[FLY_MP_MAXGAMES];			//!< GUID of each game
	
	//! Default constructor
	flyMPGames() { num=0; };			
};

//! Multiplayer player data
/*!
	This class implements a data structure for storing information concerning
	a player in the multiplayer environment. Among its members are the player's
	DirectPlay identification number, his name and characteristic color.
*/
class FLY_DIRECTX_API flyMPPlayerData
{
	public:
		DWORD	type,	//!< player type
				dpid;	//!< player's id in DirectPlay
		char name[128];	//!< player's name
		unsigned color;	//!< player's color (ARGB)
		void *data;		//!< any additional data

	//! Default constructor
	flyMPPlayerData() { memset(this,0,sizeof(flyMPPlayerData)); };
};

//! Interface with DirectInput, DirectSound and DirectPlay
/*!
	This class implements friendly methods that compound an interface with the 
	DirectX modules used by Fly3D, namely DirectInput, DirectSound and DirectPlay. 
	Among the member variables are pointers to the various DirectX devices, 
	the total number of online players and an array of player data structures.
	Various interface methods are supplied for many operations, like: loading 
	playable wave files, reading or zeroing the input buffer, initializing and 
	finishing the multiplayer mode, creating a multiplayer game, adding and 
	removing players from a game, and much more.
*/
class FLY_DIRECTX_API flyDirectx
{
public:
	// Windows
	HWND hWnd;								//!< application window handle
	HINSTANCE hInst;						//!< instance handle

	// Direct Input
	LPDIRECTINPUT           lpDI;			//!< pointer to DirectInput device
	LPDIRECTINPUTDEVICE     lpKeyboard;		//!< pointer to keyboard
	LPDIRECTINPUTDEVICE     lpMouse;		//!< pointer to mouse

#ifndef FLY_WIN_NT4_SUPORT
	LPDIRECTINPUTDEVICE     lpJoystick;		//!< pointer to joystick
	LPDIRECTINPUTDEVICE2    lpJoystickPoll;	//!< pointer to joystick poll device
#endif

	// Direct Sound
	LPDIRECTSOUND			lpDSound;		//!< pointer to DirectSound device
	LPDIRECTSOUNDBUFFER		lpDS3dPrimBuf;	//!< pointer to 3D Primary Buffer
	LPDIRECTSOUND3DLISTENER lpDS3dListener;	//!< pointer to 3D Listener

	// Direct Play
	LPDPSESSIONDESC2	lpSD;				//!< pointer to DirectPlay session descriptor
	LPDIRECTPLAY3A		lpDP;				//!< pointer to DirectPlay device
	LPDIRECTPLAYLOBBY2A	lpDPL;				//!< pointer to DirectPlay lobby
	GUID				guidInstance;		//!< instance of the GUID

	//! DirectX initialisation and member variables validation
	void Init();
	//! Free the allocated resources and release the devices
	void Release();

	int mpmode;									//!< multiplayer mode
	int nplayers;								//!< number of players
	flyMPPlayerData players[FLY_MP_MAXPLAYERS];	//!< array of players

	unsigned char keys[256];	//!< keyboard keys
	
	int mouse_pos[2];		//!< current mouse position in screen pixels
	float mouse_delta[2];		//!< mouse displacement
	float mouse_smooth[2];	//!< smooooooooooth movement position
	char mouse_down;		//!< bitfield registering which mouse buttons are down
	char mouse_click;		//!< bitfield registering which mouse buttons have been clicked
	
	static float s_mousespeed;	//!< mouse speed
	static int s_invertmousex;	//!< flag to invert mouse input in x direction
	static int s_invertmousey;	//!< flag to invert mouse input in y direction

#ifndef FLY_WIN_NT4_SUPORT
	static float s_joyspeed;	//!< joystick speed
	static int s_invertjoyx;	//!< flag to invert joystick input in x direction
	static int s_invertjoyy;	//!< flag to invert joystick input in y direction
	int joy_min_range;		//!< joy axis minium range
	int joy_max_range;		//!< joy axis maxium range
	int joy_dead_zone;		//!< joy stick dead zone (if the stick is in the dead zone the pos is returned as centered)
	int joy_axes;			//!< joysitck supported axis
	int joy_down;			//!< bitfield registering which joysticks buttons are down
	float joy_pos[2];		//!< current joystick position
	DIDEVCAPS joy_caps;		//!< joystick capabilities
#endif

	//! Default constructor, gets the window and instance handlers
	flyDirectx(HWND hwnd,HINSTANCE hinst)
	{ 
		hWnd=hwnd;
		hInst=hinst;

		lpDI=0;
		lpKeyboard=0;
		lpMouse=0;

		lpDSound=0;
		lpDS3dPrimBuf=0;
		lpDS3dListener=0;

		lpSD=0;
		lpDP=0;
		lpDPL=0;

#ifndef FLY_WIN_NT4_SUPORT
		lpJoystick=0;
		lpJoystickPoll=0;
		joy_min_range=-1000;
		joy_max_range=1000;
		joy_dead_zone=5000;
		joy_axes=0;
		joy_down=0;
		joy_pos[0]=joy_pos[1]=0;
#endif

		mouse_down=0;
		nplayers = 0;
		mpmode = 0;

		Init();
	};
	
	//! Default destructor
	virtual ~flyDirectx()
	{ 
		Release(); 
	};

	//! get user input from input devices
	void get_input();
	//! reset all input
	void zero_input();

	//! load .wav sound file
	int load_wav_file(LONG cchBuffer,HPSTR pchBuffer,LPDIRECTSOUNDBUFFER *buf,LPDIRECTSOUND3DBUFFER *buf3d);
	//! clone an existing sound
	LPDIRECTSOUNDBUFFER clone_sound(LPDIRECTSOUNDBUFFER buf);
	//! sets position, velocity and alignment of a listener
	void set_listener(const float *pos,const float *vel,const float *Y,const float *Z);
	//! set master sound volume
	void set_master_volume(int volume);

	//! initialise multiplayer session
	int init_multiplayer(const char *netaddress=0);
	//! destroy multiplayer session
	void free_multiplayer();

	//! get a pointer to the list of available games
	flyMPGames *enum_games(LPGUID app_guid);
	//! join an existing game
	int join_game(LPGUID game_guid,const char *player_name,unsigned color=0xFF808080);
	//! create (host) a new game
	int create_game(LPGUID app_guid,const char *game_name);
	//! get player IP address from its DirectPlay id
	char *get_player_address(DWORD dpid);

	//! send a message over the net
	void send_message(const flyMPMsg *msg,int len,DWORD dpid=0);
	//! get total number of messages
	int get_num_messages();
	//! get a message
	flyMPMsg *get_message(DWORD *size);

	//! add a new player to the environment
	int add_player(const char *name,DWORD dpid,void *data,unsigned color=0xFF808080);
	//! remove a p´layer from the environment
	void *remove_player(int i);
};

//! global flyDirectx instance
extern FLY_DIRECTX_API flyDirectx *g_flydirectx;
//! global DirectX initialisation method
FLY_DIRECTX_API void fly_init_directx(HWND hwnd,HINSTANCE hinst);
//! global DirectX release method
FLY_DIRECTX_API void fly_free_directx();

#endif