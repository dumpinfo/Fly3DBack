/*! \file */

//! The engine's basic object class
/*!
	This class is the base class for all objects. It holds the object's name, long name, 
	type and pointer to the next object in the specific object linked list.
*/
class FLY_ENGINE_API flyBaseObject
{
	public: 
		int type;
		flyString name;				//!< the object's name
		flyString long_name;		//!< the object's long name
		flyBaseObject *next_obj;	//!< the next object in the linked list

		//! Default constructor
		flyBaseObject();

		//! Copy-constructor
		flyBaseObject(const flyBaseObject& in);

		//! Atribuition operator
		void operator=(const flyBaseObject& in);

		//! Default destructor
		virtual ~flyBaseObject();
};
