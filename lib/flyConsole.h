/*! \file */

//! Console class
/*! 
	This class implements the console, consisting of a command line where you can execute commands. 
	Runtime information can also be printed out on the console, and the navigation keys 
	'home', 'end', 'page up' and 'page down' can be used to scroll trough messages.
*/
class FLY_ENGINE_API flyConsole
{
	public:
		int mode,		//!< display mode (mode<0: hide, mode>0: show)
			time,		//!< maximum idle display time before auto-hiding
			nlines,		//!< total number of strings in the buffer
			linecount,	//!< number of lines
			winlines,	//!< number of lines in the window
			lic;		//!< flag indicating console input lock

		char *buf[FLY_CONSOLE_MAXLINES];	//!< strings buffer

		char cmd_line[1024];	//!< input line to be written (command line)
		int cmd_pos,			//!< current cursor position
			cmd_len,			//!< command string length
			curr_line,			//!< current line index
			curr_page;			//!< current page index

		float	dx,	//!< x cursor position in screen pixels
				dy;	//!< y cursor position in screen pixels

	//! Default constructor
	flyConsole();

	//! Default destructor
	virtual ~flyConsole();

	//! Show console
	void show();
	//! Hide console
	void hide();
	//! Draw console
	void draw();
	//! Update console status each frame
	void step(int dt);
	//! Draw text of the current page and command line
	float draw_text(float x,float y,const char *text) const;

	//! Get input from navigation keys
	void key_down(int vk);
	//! Get input from ordinary text keys
	void key_char(int vk);

	//! Add a string to the console
	void add_string(const char *fmt, ...);
	//! Lexical analyser
	int command_tokens(char *str,char **token) const;
	//! Execute the command
	void command_exec(const char *str);
};
