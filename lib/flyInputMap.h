/*! \file */

//! Class representing input maps
/*!
	This class implements a Input Map, i.e. a data structure that maps keyboard keys and mouse buttons
	into game actions. Its members are the name of the input map, the keyboard key and the mouse buttons 
	assigned to it.
*/
class FLY_ENGINE_API flyInputMap
{
	public: 
		flyString name;
		int key;
		int mouse;
		int joystick;
		int is_clicked;

		//! Default constructor
		flyInputMap();

		//! Copy-constructor
		flyInputMap(const flyInputMap& in);
		
		//! Atribuition operator
		void operator=(const flyInputMap& in);

		//! Default destructor
		virtual ~flyInputMap();

		int check_input();
		int check_clicked_input();
};
