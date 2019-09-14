/*! \file */

//! Node of the BSP tree
/*!
	This class implements a node from the bsp tree. If it is a leaf node, then it defines a convex
	volume in 3D space. The class members are: pointers to children nodes, an array of flyBspObjects
	that clip the volume, an array of neighbouring nodes, the position of the center of the node and 
	the node's natural ambient light color.
*/
class FLY_ENGINE_API flyBspNode : public flyPlane
{
	public:
		flyBspNode *child[2];				//!< the node's children
		flyArray<flyBspObject *> elem;		//!< the clipping elements array (if this node is a leaf)
		flyArray<flyBspNode *> neighbors;	//!< the node's neighbors
		flyArray<flyPolygon> portals;		//!< linking polygons to the neighbors
		flyVector centre;					//!< the centre of the node (if this node is a leaf)

		int leaf;			//!< leaf index (-1 if not a leaf)
		flyVector color;	//!< node's ambient light

	//! Default constructor
	flyBspNode();

	//! Copy-constructor
	flyBspNode(const flyBspNode& in);

	//! Atribuition operator
	void operator=(const flyBspNode& in);

	//! Default destructor
	virtual ~flyBspNode();

	//! Add a new neighbor node and its connecting portal polygon
	void add_neighbor(const flyPolygon& p,flyBspNode* in);
};
