/*! \file */

//! Picture image class
/*!
	This class implements a picture with 24 or 32 bits/pixel. 
	The picture pixels are allocated in a single array of bytes (buf[0]) 
	and the other variables are just shortcuts to the picture lines (buf[1] to buf[sy-1]).
*/
class FLY_RENDER_API flyPicture
{
	public:
		char name[256];		//!< picture filename
		int sx,				//!< x size in pixels 
			sy,				//!< y size in pixels 
			bytespixel,		//!< number of bytes per pixel (24 or 32)
			size;			//!< size in bytes for the image (sx*sy*bytespixel)
		unsigned char *buf;	//!< image pixels 

	//! Default constructor
	flyPicture() :
		buf(0),sx(0),sy(0),
		bytespixel(0),size(0)
	{ 	
		name[0]=0; 
	}

	//! Copy-constructor
	flyPicture(flyPicture& in);

	//! atribuition operator
	void operator=(const flyPicture& in);

	//! Default Destructor
	virtual ~flyPicture()
	{ FreePicture(); }

	//! Load a .jpg image
	int  LoadJPG(const unsigned char *data,int len,int flipy=0);
	//! Load a .tga image. Only 24 or 32 bits/pixel images are supported, uncompressed or RLE compressed
	int  LoadTGA(const unsigned char *data,int len,int flipy=0);
	//! Save a .tga image
	int  SaveTGA(const char *file);
	//! Save a .jpg image at specified quality factor (0-100)
	int  SaveJPG(const char *file,int quality=85);
	//! Save a .jpg image at specified quality factor (0-100)
	int  SaveJPG(FILE *fp,int quality=85);
	//! Allocate the memory required for the a 32-bit picture with the specified dimensions. The buffer allocated is xc*yd*4 bytes
	void CreatePicture32(int xd,int yd);
	//! Allocate the memory required for the a 24-bit picture with the specified dimensions. The buffer allocated is xc*yd*3 bytes
	void CreatePicture24(int xd,int yd);
	//! Resamples the image to the closest power of 2 dimensions
	void CheckSize(int maxlevel);
	//! Free all data allocated by the picture
	void FreePicture(void);
	//! Flips picture in Y axis (first line turns to last line)
	void FlipY();
};

