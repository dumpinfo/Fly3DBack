#include "../flyRender.h"

flyVideo::flyVideo()
{
	hdc=CreateCompatibleDC(0);
	data=0;
	texture=0;
}

flyVideo::~flyVideo()
{
	if (data)
	{
		DeleteObject(hb);
		DrawDibClose(hdd);
		AVIStreamGetFrameClose(pgf);
		AVIStreamRelease(pavi);
		AVIFileExit();
	}
}

int flyVideo::load_avi(const char *file)
{
	if (data)
	{
		DeleteObject(hb);
		DrawDibClose(hdd);
		AVIStreamGetFrameClose(pgf);
		AVIStreamRelease(pavi);
		AVIFileExit();
	}
	data=0;
	curframe=-1;
	lasttime=0;

	if (flyRender::s_videores==0)
		return 0;

	resolution=1<<flyRender::s_videores;

	hdd=DrawDibOpen();
	AVIFileInit();
	if(AVIStreamOpenFromFile(&pavi, file, streamtypeVIDEO, 0, OF_READ, NULL))
	{
		DrawDibClose(hdd);
		AVIFileExit();
		return 0;
	}

	AVIStreamInfo(pavi, &psi, sizeof(psi));

	width=psi.rcFrame.right - psi.rcFrame.left;
	height=psi.rcFrame.bottom - psi.rcFrame.top;

	numframes=AVIStreamLength(pavi);

	frametime=AVIStreamSampleToTime(pavi, numframes)/numframes;

	bih.biSize			= sizeof(BITMAPINFOHEADER);
	bih.biPlanes		= 1;
	bih.biBitCount		= 24;
	bih.biWidth			= resolution;
	bih.biHeight		= resolution;
	bih.biCompression	= BI_RGB;

	hb = CreateDIBSection(hdc, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void **) &data, NULL, NULL);
	SelectObject(hdc, hb);

	pgf=AVIStreamGetFrameOpen(pavi, NULL);
	if(!pgf)
	{
		DeleteObject(hb);
		DrawDibClose(hdd);
		AVIStreamRelease(pavi);
		AVIFileExit();
		data=0;
		return false;
	}

	texture=g_flytexcache->add_tex(file,this,FLY_TEXFLAG_FILTER|FLY_TEXFLAG_TRUECOLOR|FLY_TEXFLAG_BGR);
	set_frame(0);

	return true;
}

void flyVideo::set_frame(int frame)
{
	if (data && texture)
	if (curframe!=frame && frame>=0 && frame<numframes)
	{
		curframe=frame;

		LPBITMAPINFOHEADER lpbi;
		lpbi=(LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);

		char *framedata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed*sizeof(RGBQUAD);
		DrawDibDraw(hdd, hdc, 0, 0, resolution, resolution, lpbi, framedata, 0, 0, width, height, 0);

		g_flytexcache->update_tex(texture, resolution, resolution, 3, data);
	}
}

void flyVideo::update(int curtime)
{
	if (data)
	{
		if (lasttime==0)
		{
			lasttime=curtime;
			set_frame(0);
		}
		else
		{
			int dt=curtime-lasttime;
			int frame=(dt/frametime)%numframes;
			set_frame(frame);
		}
	}
}
