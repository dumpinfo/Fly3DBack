#include "../Fly3D.h"

flyInputMap::flyInputMap() : 
	key(-1),mouse(0),joystick(0),is_clicked(0)
{ 
}

flyInputMap::flyInputMap(const flyInputMap& in) :
	name(in.name),
	key(in.key),
	mouse(in.mouse),
	joystick(in.joystick),
	is_clicked(in.is_clicked)
{
}

void flyInputMap::operator=(const flyInputMap& in) 
{
	name=in.name;
	key=in.key;
	mouse=in.mouse;
	joystick=in.joystick;
	is_clicked=in.is_clicked;
}

flyInputMap::~flyInputMap() 
{
}

int flyInputMap::check_input()
{
	if (key!=-1 && g_flydirectx->keys[key])
		return true;
	if (g_flydirectx->mouse_down&mouse)
		return true;
#ifndef FLY_WIN_NT4_SUPORT
	if (g_flydirectx->joy_down&joystick)
		return true;
#endif
	return false;
}

int flyInputMap::check_clicked_input()
{
	if(check_input())
	{
		if(is_clicked)
			return false;

		is_clicked=1;
		return true;
	}
	else
	{
		is_clicked=0;
		return false;
	}
}