/*! \file */

//! Sound class
/*!
	This class implements raw sound data that can be loaded from a .wav file. The class members 
	are DirectSound buffers and the total length of the sound file. The methods include loading .wav 
	files from the disk and instantiating and freeing a sound
*/
class FLY_ENGINE_API flySound : public flyBaseObject
{
	public:
		int total_time;	//!< sound length in miliseconds
 		
		LPDIRECTSOUNDBUFFER file_buf;		//!< the DirectSound sound file buffer
		LPDIRECTSOUND3DBUFFER file_buf3d;	//!< the DirectSound 3D sound file buffer
		LPDIRECTSOUNDBUFFER file_buf2d;		//!< the DirectSound 2D sound file buffer

		flyArray<LPDIRECTSOUNDBUFFER> buf;		//!< array of sound buffers
		flyArray<LPDIRECTSOUND3DBUFFER> buf3d;	//!< array of 3D sound buffers

	//! Default constructor
	flySound();

	//! Default destructor
	virtual ~flySound();

	//! Free all data
	void reset();
	//! Load a wave (.wav) file
	int load_wav(const char *filename);
	//! Get an instance of the sound
	int get_sound_instace(int flag3d);
	//! Free an instance of the sound
	void free_sound_instance(int i);
};
