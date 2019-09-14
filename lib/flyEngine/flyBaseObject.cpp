#include "../Fly3D.h"

flyBaseObject::flyBaseObject() : 
	type(0),
	next_obj(0) 
{ 
}

flyBaseObject::~flyBaseObject() 
{ 
	delete next_obj; 
}

flyBaseObject::flyBaseObject(const flyBaseObject& in) :
	type(in.type),
	name(in.name),
	long_name(in.long_name),
	next_obj(0)
{ 
}

void flyBaseObject::operator=(const flyBaseObject& in) 
{
	type=in.type;
	name=in.name;
	long_name=in.long_name;
	next_obj=0;
}

