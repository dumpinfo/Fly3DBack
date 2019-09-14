/*! \file */

//! Video class
/*!
	This class implements an avi video that plays to texture map. 
	The video if converted from any format to a 256x256 texture 
	that can be used to map any 3d mesh and is updated only when visible.
*/
class FLY_RENDER_API flyVideo
{
	HDC hdc;
	int lasttime;

	AVISTREAMINFO psi;
	PAVISTREAM pavi;
	PGETFRAME pgf;
	
	BITMAPINFOHEADER bih;
	HBITMAP hb;
	HDRAWDIB hdd;
		
	public:
		int curframe;			//!< current frame
		int numframes;			//!< total number of frames in
		int width;				//!< x size in pixels 
		int height;				//!< y size in pixels 
		int resolution;			//!< texture resolution (power of 2)
		int frametime;			//!< time in ms for each frame
		unsigned char *data;	//!< current frame bitmap
		unsigned texture;		//!< current frame texture id

	//! default constructor
	flyVideo();
	//! destructor
	virtual ~flyVideo();

	//! loads an avi file
	int load_avi(const char *file);
	//! set current frame
	void set_frame(int frame);
	//! apdate animation frame
	void update(int curtime);
};
