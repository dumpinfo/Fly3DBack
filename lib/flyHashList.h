/*! \file */

//! Hash of flyBaseObject linked lists
/*!
	This class implements a hash table of flyBaseObject derived classes.
	The hash is organized by class type and for each class type a list of 
	flyBaseObject derived classes is available.
*/
class FLY_ENGINE_API flyHashList
{
	public:
		flyArray<int> m_hash;				//!< the primary hash list, ordered by object type
		flyArray<flyBaseObject *> m_list;	//!< the secondary hash lists
		flyArray<flyBaseObject *> m_last;	//!< the last objects of the lists
		int count;

	//! Default constructor
	flyHashList();
	
	//! Default destructor
	virtual ~flyHashList();

	//! Resets the whole list
	void reset();

	//! Add an object to the list
	void add(flyBaseObject *obj);
	//! Remove an object from the list
	void remove(flyBaseObject *obj);

	//! Retrieve an object with the given name
	flyBaseObject *get_object_name(const char *name,int type=0) const;
	//! Retrieve an object with the given long name
	flyBaseObject *get_object_longname(const char *long_name,int type=0) const;
	//! Get the object that follows the given one
	flyBaseObject *get_next_object(const flyBaseObject *o,int type=0) const;
};
