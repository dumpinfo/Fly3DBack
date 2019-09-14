/*! \file */

//! Class description meta-class
/*!
	This meta-class implements a class description for plugin management and objects instantiation 
	and referencing. Any plugin class that wants to be recognized and referenced by the engine must 
	have a child class from this one, implementing that class' description. It consists of three 
	methods: one for creating a new instanmce of the described class; one for retrieving the class
	name; and one for retrieving the integer that represents the class type.
*/
class FLY_ENGINE_API flyClassDesc
{
	public:
		//! Instantiate an object of this class
		virtual flyBspObject *create()=0;
		//! Get the class' name
		virtual const char *get_name()=0;
		//! Get the class' type
		virtual int get_type()=0;
		//! Destructor
		virtual ~flyClassDesc()
		{ }
};

//! Parameter description class
/*!
	This class holds information on Fly3D plugin object parameters. Each class defined in a Fly3D 
	plugin (derived from flyBspObject) can have any number of parameters. Each parameter must have 
	an associated flyParamDesc class describing its data. The flyParamDesc class encloses the 
	parameter's name, type and value, and is very useful in parameter-editing front-ends, like
	flyEditor.
*/
class FLY_ENGINE_API flyParamDesc
{
	public:
		flyString name;	//!< parameter name
		int type;		//!< parameter type
		void *data;		//!< parameter value
	
	//! Get a string
	const char *get_string() const;
	//! Set a string
	void set_string(const char *str);
	//! Destructor
	virtual ~flyParamDesc()
	{ }
};
