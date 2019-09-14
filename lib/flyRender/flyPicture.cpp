#include "../flyRender.h"
#include "ijl.h"

flyPicture::flyPicture(flyPicture& in) :
	sx(in.sx),sy(in.sy),
	bytespixel(in.bytespixel),
	size(in.size)
{ 
	buf=new unsigned char[size];
	memcpy(buf,in.buf,size);
}

void flyPicture::operator=(const flyPicture& in)
{
	FreePicture();

	sx=in.sx;
	sy=in.sy;
	bytespixel=in.bytespixel;
	size=size;
	strcpy(name,in.name);

	if (size)
	{
		buf=new unsigned char[size];
		memcpy(buf,in.buf,size);
	}
}

void flyPicture::FreePicture(void)
{
	delete[] buf;
	buf=0;
}

void flyPicture::CreatePicture32(int xd,int yd)
{
    sx=xd; sy=yd; bytespixel=4;
	size=sx*sy*bytespixel;

	buf=new unsigned char[size];
}

void flyPicture::CreatePicture24(int xd,int yd)
{
    sx=xd; sy=yd; bytespixel=3;
	size=sx*sy*bytespixel;

	buf=new unsigned char[sx*sy*bytespixel];
}

void flyPicture::CheckSize(int droplevel)
{
	int levelx=-1,levely=-1;
	int newsx,newsy;
	int i,j,k;

	i=1;
	while(levelx==-1)
	{
		j=1<<i;
		if (sx==j)
			levelx=i;
		else if (sx<j)
			levelx=i-1;
		i++;
	}
	i=1;
	while(levely==-1)
	{
		j=1<<i;
		if (sy==j)
			levely=i;
		else if (sy<j)
			levely=i-1;
		i++;
	}

	levelx-=droplevel;
	levely-=droplevel;

	if (levelx<0)
		levelx=0;
	if (levely<0)
		levely=0;
	newsx=1<<levelx;
	newsy=1<<levely;
	if (newsx==sx && newsy==sy)
		return;

	char str[256];
	sprintf(str,"resize texture %s from (%i,%i) to (%i,%i)\n",name,sx,sy,newsx,newsy);
	OutputDebugString(str);

	unsigned char *data=new unsigned char [newsx*newsy*bytespixel];
	int x,y,xx,yy,xy,p=0,rgba[4];
	for( y=0;y<newsy;y++ )
	for( x=0;x<newsx;x++ )
	{
		xx=(x+1)*sx/newsx;
		yy=(y+1)*sy/newsy;
		xy=(xx-x*sx/newsx)*(yy-y*sy/newsy);
		rgba[0]=rgba[1]=rgba[2]=rgba[3]=0;
		for( j=y*sy/newsy;j<yy;j++ )
			for( i=x*sx/newsx;i<xx;i++ )
				for( k=0;k<bytespixel;k++ )
					rgba[k]+=buf[(j*sx+i)*bytespixel+k];
		
		for( k=0;k<bytespixel;k++ )
			data[p++]=rgba[k]/xy;
	}
	
	delete[] buf;
	buf=data;
	sx=newsx;
	sy=newsy;
	size=sx*sy*bytespixel;
}

void flyPicture::FlipY()
{
	unsigned char *line=new unsigned char[bytespixel*sx];
	for( int i=0;i<sy/2;i++ )
		{
		memcpy(line,&buf[i*sx*bytespixel],bytespixel*sx);
		memcpy(&buf[i*sx*bytespixel],&buf[(sy-i-1)*sx*bytespixel],bytespixel*sx);
		memcpy(&buf[(sy-i-1)*sx*bytespixel],line,bytespixel*sx);
		}
	delete[] line;
}

int flyPicture::LoadJPG(const unsigned char *data,int len,int flipy)
{
	JPEG_CORE_PROPERTIES jpg_data;
	int rslt=-1;

	FreePicture();

	memset(&jpg_data,0, sizeof(JPEG_CORE_PROPERTIES));

	if(ijlInit(&jpg_data)!=IJL_OK)
		return 0;

	jpg_data.JPGBytes=(unsigned char *)data;
	jpg_data.JPGSizeBytes=len;

	if(ijlRead(&jpg_data, IJL_JBUFF_READPARAMS)==IJL_OK)
		if(jpg_data.JPGChannels==3)
		{
			jpg_data.DIBColor=IJL_RGB;
			jpg_data.DIBWidth=jpg_data.JPGWidth;
			jpg_data.DIBHeight=jpg_data.JPGHeight;
			jpg_data.DIBChannels=jpg_data.JPGChannels;
			CreatePicture24(jpg_data.DIBWidth, jpg_data.DIBHeight);
			jpg_data.DIBBytes = buf;
			rslt=ijlRead(&jpg_data, IJL_JBUFF_READWHOLEIMAGE);
		}
	
	ijlFree(&jpg_data);

	if (flipy)
		FlipY();

	return rslt==IJL_OK;
}

int flyPicture::LoadTGA(const unsigned char *data,int len,int flipy)
{
	int x_pos1, x_pos2, y, i, p, cursize;
	unsigned char flag, pixel_order;
	int filepos=18;

	FreePicture();

	sx=sy=0;
		
	if(!((data[16]==24 || data[16]==32) && (data[2]==2 || data[2]==10)))
		return 0;

	sx=*((unsigned short *)&data[12]);
	sy=*((unsigned short *)&data[14]);

	if(data[16]==24)
		CreatePicture24(sx,sy);
	else
		CreatePicture32(sx,sy);

	pixel_order=data[17]&0x30;

	if(data[2]==2)
		memcpy(buf,&data[filepos],size);
	else
	{
		unsigned char *p=buf, *q, c;

		cursize=0;
		while(cursize<sx*sy)
		{
			c=data[filepos++];
			if(!(c&0x80))
			{
				c++;
				memcpy(p,&data[filepos],c*bytespixel);
				filepos+=c*bytespixel;
				p+=c*bytespixel;
			}
			else
			{
				c=(c&0x7f)+1;
				memcpy(p,&data[filepos],bytespixel);
				filepos+=bytespixel;
				q=p;
				for(i=1; i<c; i++)
				{
					q+=bytespixel;
					q[0]=p[0];
					q[1]=p[1];
					q[2]=p[2];
				}
				p=q;
				p+=bytespixel;
			}
			cursize+=c;
		}
	}

	if (flipy)
	{
		if (pixel_order&0x20)
			FlipY();
	}
	else
		if ((pixel_order&0x20)==0)
			FlipY();

	for( y=0;y<sy;y++ )
	{
		x_pos1=0;
		for(p=0; p<sx; p++)
		{
			flag=buf[y*sx*bytespixel+x_pos1+2];
			buf[y*sx*bytespixel+x_pos1+2]=buf[y*sx*bytespixel+x_pos1];
			buf[y*sx*bytespixel+x_pos1]=flag;
			x_pos1+=bytespixel;
		}
		if(pixel_order&0x10)
		{
			x_pos1=0;
			x_pos2=(sx-1)*bytespixel;
			for(p=0; p<sx/2; p++)
			{
				for(i=0; i<bytespixel; i++)
				{
					flag=buf[y*sx*bytespixel+x_pos2+i];
					buf[y*sx*bytespixel+x_pos2+i]=buf[y*sx*bytespixel+x_pos1+i];
					buf[y*sx*bytespixel+x_pos1+i]=flag;
				}
				x_pos1+=bytespixel;
				x_pos2-=bytespixel;
			}
		}
	}

	return 1;
}

int flyPicture::SaveTGA(const char *file)
{
    FILE *fp;
    int a,b;
    unsigned char TGA_INI[18];
    unsigned char *picline;

    picline=new unsigned char[sx*bytespixel];
    if (!picline)
       return 0;

    if ((fp=fopen(file,"wb"))!=0)
       {
        memset(&TGA_INI[0],0,18);
        TGA_INI[12]=(unsigned char)(sx%256);
        TGA_INI[13]=(unsigned char)(sx/256);
        TGA_INI[14]=(unsigned char)(sy%256);
        TGA_INI[15]=(unsigned char)(sy/256);
        TGA_INI[2]=2;
        TGA_INI[16]=8*bytespixel;
		TGA_INI[17]=0x20;
        fwrite((char *)&TGA_INI[0],18,1,fp);
        for( a=sy-1;a>=0;a-- )
             {
              for( b=0;b<sx;b++ )
                   {
                    picline[b*bytespixel]=buf[(a*sx+b)*bytespixel+2];
                    picline[b*bytespixel+1]=buf[(a*sx+b)*bytespixel+1];
                    picline[b*bytespixel+2]=buf[(a*sx+b)*bytespixel];
					if (bytespixel==4)
						picline[b*bytespixel+3]=buf[(a*sx+b)*bytespixel+3];
                   }
              if (fwrite((char *)picline,sx,bytespixel,fp)!=(unsigned)bytespixel)
                 {
                  fclose(fp);
                  delete[] picline;
                  return 0;
                 }
             }
        fclose(fp);
       }
    delete[] picline;
    return 1;
}

int flyPicture::SaveJPG(const char *file,int quality)
{
	if (buf==0 || file==0)
		return 0;

	FILE *fp=fopen(file,"wb");
	if (fp==0)
		return 0;

	int rslt=SaveJPG(fp,quality);

    fclose(fp);
	
	return rslt;
}

int flyPicture::SaveJPG(FILE *fp,int quality)
{
	if (buf==0 || fp==0)
		return 0;

	JPEG_CORE_PROPERTIES jpg_data;

	memset(&jpg_data,0, sizeof(JPEG_CORE_PROPERTIES));

	if(ijlInit(&jpg_data)!=IJL_OK)
		return 0;

	unsigned char *jpg_buf=new unsigned char[sx*sy*3];

    jpg_data.DIBWidth       = sx;
    jpg_data.DIBHeight      = sy;
    jpg_data.DIBBytes       = buf;
    jpg_data.DIBPadBytes    = 0;
    jpg_data.DIBChannels    = 3;
    jpg_data.DIBColor       = IJL_RGB;

    jpg_data.JPGWidth       = sx;
    jpg_data.JPGHeight      = sy;
    jpg_data.JPGFile        = 0;
    jpg_data.JPGBytes       = jpg_buf;
    jpg_data.JPGSizeBytes   = sx*sy*3;
    jpg_data.JPGChannels    = 3;
    jpg_data.JPGColor       = IJL_YCBCR;
    jpg_data.JPGSubsampling = IJL_411;
    jpg_data.jquality       = quality;

	int rslt = ijlWrite(&jpg_data,IJL_JBUFF_WRITEWHOLEIMAGE);

	ijlFree(&jpg_data);

	fwrite(jpg_buf,jpg_data.JPGSizeBytes,1,fp);

	delete jpg_buf;

	return rslt==IJL_OK;
}
