/*! \file */

//! Fly3D plugin DLL class
/*!
	This class holds information for each Fly3D plugin DLL, including the number of classes implemented 
	in the DLL, and the pointers to the DLL exported functions.
*/
class FLY_ENGINE_API flyDll
{
	public:
		HINSTANCE hdll;			//!< instance handle of the plugin DLL
		flyString dll_filename;	//!< file name of the plugin DLL
		int nclasses;			//!< number of classes implemented in the plugin

	//! Default constructor
	flyDll();

	//! Default destructor
	virtual ~flyDll();

	//! Pointer to the 'num_classes' function of the plugin
	int (* num_classes)();
	//! Pointer to the 'fly_message' function of the plugin
	int (* fly_message)(int msg,int param,void *data);
	//! Pointer to the 'get_version' function of the plugin
	int (* get_version)();
	//! Pointer to the 'get_class_desc' function of the plugin
	flyClassDesc *(* get_class_desc)(int i);
	//! Pointer to the 'get_global_param_desc' function of the plugin
	int (* get_global_param_desc)(int i,flyParamDesc *pd);
};

//! DLLs grouping class
/*!
	This class implements a group of Fly3D plugin DLLs. 
	Each DLL can enumerate any number of flyBspObject derived classes. 
*/
class FLY_ENGINE_API flyDllGroup
{
	public:
		int ndll;		//!< number of DLLs in the group
		flyDll **dll;	//!< list of DLLs in the group

		int ncd;			//!< total number of classes implemented in all DLLs in the group
		flyClassDesc **cd;	//!< list of all classes from all the DLLs in the group

	//! Default constructor
	flyDllGroup();

	//! Default destructor
	virtual ~flyDllGroup();
	
	//! Free all data
	void reset();
	
	//! Add a new DLL to the group, given its file name
	int add_dll(const char *filename);
	//! Delete a DLL from the group, given its file name
	int delete_dll(const char *filename);
	//! Move a DLL inside the group to the desired new relative position 'newrelpos'
	void move_dll(const char *filename,int newrelpos);

	//! Send a message to all plugin DLLs in the group
	int send_message(int msg,int param,void *data) const;
	
	//! Load all classes used by the given .fly file
	void load_all_classes(flyFile *file);
	//! Load global parameters from the plugin DLLs specified by 'd'
	void load_params(int d,flyFile *file);
	//! Load all classes from the plugin DLLs specified by 'd'
	void load_classes(int d,flyFile *file);
	//! Load all the default parameters for the specified object 'o'
	void load_default_param(flyBspObject *o,const char *sec,flyFile *file);
	//! Delete an object from the scene, along with all references to it
	void delete_class(flyBspObject *o);
	//! Delete all the references of the given object 'o'
	void delete_references(flyBspObject *o);
	//! Create a new object from a plugin DLL through its class name
	flyBspObject *add_class(const char *name);
};
