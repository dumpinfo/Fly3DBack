#include "../Fly3D.h"

flySound::flySound() : 
	total_time(0), 
	file_buf(0),
	file_buf3d(0),
	file_buf2d(0)
{ 
	type=FLY_TYPE_SOUND; 
}

flySound::~flySound()
{ 
	reset(); 
}

void flySound::reset()
{
	int i;
	for( i=0;i<buf3d.num;i++ )
		if (buf3d[i]) 
			buf3d[i]->Release(); 
	for( i=0;i<buf.num;i++ )
		if (buf[i]) 
			buf[i]->Release(); 
	buf.free();
	buf3d.free();
	if (file_buf)
		file_buf->Release();
	file_buf=0;
	total_time=0; 
}

int flySound::load_wav(const char *filename)
{
	if (g_flydirectx==0)
		return 0;

	flyFile f;

	if (!f.open(filename)) 
		return 0;

	reset();

	total_time=g_flydirectx->load_wav_file(f.len,(char *)f.buf,&file_buf2d,0);
	total_time=g_flydirectx->load_wav_file(f.len,(char *)f.buf,&file_buf,&file_buf3d);

	return total_time;
}

int flySound::get_sound_instace(int flag3d)
{
	if (file_buf==0)
		return -1;

	int i;
	for( i=0;i<buf.num;i++ )
		if (buf[i]==0)
			break;

	if (i==buf.num)
	{
		buf.add(0);
		buf3d.add(0);
	}

	if (flag3d)
		buf[i]=g_flydirectx->clone_sound(file_buf);
	else
		buf[i]=g_flydirectx->clone_sound(file_buf2d);

	if (buf[i]==0)
		return -1;

	if (flag3d)
		buf[i]->QueryInterface(IID_IDirectSound3DBuffer,(void **)(&buf3d[i]));

	return i;
}

void flySound::free_sound_instance(int i)
{
	if (i<buf.num && buf[i]!=0)
	{
		if (buf3d[i])
			buf3d[i]->Release();
		if (buf[i])
			buf[i]->Release();
		buf[i]=0;
		buf3d[i]=0;
	}		
}

