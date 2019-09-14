/*! \file */

//! Fly3D scene data file management class
/*!
	This class implements a manager and interface for Fly3D's scene data files (.fly files).
	Its members are a pointer to a data buffer containing the file load on memory, the current
	access position in the file and the total length of the file. Among the methods available 
	in this class are: opening and closing a file, seeking a position in it, unzipping a zipped 
	file, reading a certain amount of data from the file, and more.
*/
class FLY_ENGINE_API flyFile
{
	void build_ziplist_rec(flyString path);

	public:
		unsigned char *buf;		//!< the file data buffer
		int pos,				//!< current position in the file
			len;				//!< length of the file
		//! List of all files included in all zip packs inside the current data folder
		static flyStringArray2 *ziplist;
		//! Table with text file itens as an array of array of strings
		//! Each group (named like '[groupname]') has an array of 
		//! parameter  strings (in the form 'param=data')
		flyStringArray2 *table;

	//! Default constructor
	flyFile();

	//! Default destructor
	virtual ~flyFile();

	//! Build the zip list with all packed files 
	void build_ziplist(flyString path);
	//! Build a array or array of strings for fast text reads of fly files
	void build_table();
	//! Unzip a zipped file
	void unzip(const char *);
	//! Seek a position in the file
	void seek(int offset);
	//! Open a file by its given file name
	bool open(const char *filename);
	//! Close the file
	void close();
	//! Read 'size' bytes from the file (beginning at the current position 'pos') into the 'dest' buffer
	int read(void *dest, int size);
	//! Get a string from the profile
	int get_profile_string(const char *section, const char *key,flyString& dest);
	//! Get a string from the file
	void get_string(flyString& dest);
	//! Get an integer from the file
	int get_int();
	//! Get an floating point from the file
	float get_float();
};
