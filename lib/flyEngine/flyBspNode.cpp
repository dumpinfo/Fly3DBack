#include "../Fly3D.h"

flyBspNode::flyBspNode() :
	leaf(-1),color(0),centre(0)
{ 
	child[0]=child[1]=0; 
}

flyBspNode::~flyBspNode()
{
	if (elem.num && elem[0]->type==FLY_TYPE_STATICMESH)
		delete elem[0];
	delete child[0];
	delete child[1];
}

void flyBspNode::operator=(const flyBspNode& in)
{
	flyPlane::operator=(in);
	child[0]=in.child[0]; child[1]=in.child[1];
	leaf=in.leaf;
	color=in.color;
	elem=in.elem;
	neighbors=in.neighbors;
	portals=in.portals;
	centre=in.centre;
}

flyBspNode::flyBspNode(const flyBspNode& in) :
	flyPlane(in),
	leaf(in.leaf),
	color(in.color),
	elem(in.elem),
	neighbors(in.neighbors),
	portals(in.portals),
	centre(in.centre)
{  
	child[0]=in.child[0]; 
	child[1]=in.child[1]; 
}

void flyBspNode::add_neighbor(const flyPolygon& p,flyBspNode* in)
{	
	portals.add(p);
	neighbors.add(in);	
}
