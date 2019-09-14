#include "../Fly3D.h"
#include <direct.h>

flyStringArray2 *flyFile::ziplist=0;

flyFile::flyFile() :
	pos(0),len(0),buf(0),table(0)
{ 
}

flyFile::~flyFile()
{ 	
	delete table;
	delete[] buf;
}

bool flyFile::open(const char *filename)
{
	close();

	FILE *fp=fopen(filename, "rb");
	if(!fp)
	{
		if(ziplist==0)
		{
			build_ziplist(g_flyengine->flysdkdatapath);
			int i,j=0;
			for( i=0;i<ziplist->num;i++ )
				j+=(*ziplist)[i].num-1;
			g_flyengine->console_printf("Zip Search: found %i file(s) in %i zip pack(s).",j,ziplist->num);
			if (ziplist->num==0)
				ziplist->num=-1;
		}

		flyString str=filename;
		int p;
		while((p=str.find('/'))!=-1) 
			str.set_char(p,'\\');

		int i,j;
		for(i=0;i<ziplist->num;i++)
		{
			flyString str2=(*ziplist)[i][0];
			if((p=str2.find_reverse('\\'))!=-1) 
				str2.set_char(p+1,0);
		
			if(strncmp(str,str2,p+1)==0)
			{
				char *c=((char *)(const char *)str)+p+1;
				 
				for(j=1;j<(*ziplist)[i].num;j++)
					if((*ziplist)[i][j].compare_nocase(c)==0)
						break;
				
				if(j==(*ziplist)[i].num) 
					continue;

				unzFile zipfile=unzOpen((*ziplist)[i][0]);

				while((p=str.find('\\'))!=-1) 
					str.set_char(p,'/');
				if(unzLocateFile(zipfile, c, 0)!=UNZ_OK)
				{
					unzClose(zipfile);
					return false;
				}

				unz_file_info pfile_info;
				unzGetCurrentFileInfo(zipfile, &pfile_info, 0, 0, 0, 0, 0, 0);
				len=pfile_info.uncompressed_size;
				
				unzOpenCurrentFile(zipfile);
				buf=new unsigned char[len];
				unzReadCurrentFile(zipfile, buf, len);
				unzCloseCurrentFile(zipfile);

				unzClose(zipfile);
				
				pos=0;
				return true;
			}
		}
	}
	else
	{
		fseek(fp, 0, SEEK_END);
		len=ftell(fp);
		buf=new unsigned char[len+1];
		buf[len]=0;
		fseek(fp, 0, SEEK_SET);
		fread(buf, len, 1, fp);
		fclose(fp);
		pos=0;
		return true;
	}

	return false;
}

void flyFile::close()
{
	delete[] buf;
	buf=0;
	pos=0;
	len=0;
}

int flyFile::read(void *dest, int size)
{
	if(pos+size>=len) size=len-pos;
	memcpy(dest, &buf[pos], size);
	pos+=size;
	return size;
}

void flyFile::get_string(flyString& dest)
{
	int i=pos;
	while(i<len && buf[i]!='\r' && buf[i]!='\n') 
		i++;

	dest.copy((char *)buf,pos,i-pos);
	
	pos=i;
	if(pos<len && buf[pos]=='\r') pos++;
	if(pos<len && buf[pos]=='\n') pos++;
}

int flyFile::get_profile_string(const char *section,const char *key,flyString& dest)
{
	int i,j,k;

	static int last=0;
	if(table==0)
	{
		build_table();
		last=0;
	}

	k=strlen(key);
	
	for(i=0;i<table->num;i++)
	{
		j=(last+i)%table->num;
		if(stricmp((*table)[j][0],section)==0)
			break;
	}

	if (i<table->num)
	{
		i=j;
		for(j=1;j<(*table)[i].num;j++)
			if (strchr((*table)[i][j],'=')-(*table)[i][j]==k && 
				strnicmp((*table)[i][j],key,k)==0)
				{
					dest=(*table)[i][j].right((*table)[i][j].length()-k-1);
					last=i;
					return 1;
				}
	}

	last=0;
	dest="";
	return 0;
}

void flyFile::build_table()
{
	delete table;
	table=new flyStringArray2;

	char *c1,*c2;
	flyString s;
	flyArray<flyString> section;

	c1=(char *)buf;
	while((c1-(char *)buf)<len)
	{
		if(*c1==10)
			c1++;
		if ((c1-(char *)buf)>=len)
			break;

		c2=strchr(c1,13);
		if(c2==0)
			c2=&c1[strlen(c1)];
		*c2=0;

		if(*c1=='[' && *(c2-1)==']')
		{
			if(section.num>0)
				table->add(section);

			section.clear();
			section.add(flyString(c1,1,c2-c1-2));
		}
		else
			if(strchr(c1,'='))
				section.add(flyString(c1,0,c2-c1));

		c1=c2+1;
	}

	if(section.num>0)
		table->add(section);
}

void flyFile::build_ziplist(flyString path)
{
	delete ziplist;
	ziplist=new flyStringArray2;
	build_ziplist_rec(path);
}

void flyFile::build_ziplist_rec(flyString path)
{
	flyString str,str2;
	int p;

	while((p=path.find('/'))!=-1) path.set_char(p,'\\');
	
	str2=path;
	str2+="*";

	HANDLE hFindFile;
	WIN32_FIND_DATA FindFileData;
	hFindFile=FindFirstFile(str2, &FindFileData);
	if(hFindFile!=INVALID_HANDLE_VALUE)
	do 
	{
		flyStringArray a;

		str=path;
		str2=FindFileData.cFileName;
		
		while((p=str2.find('/'))!=-1) str2.set_char(p,'\\');

		if(str2.compare(".")==0 || str2.compare("..")==0)
			continue;

		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			str2+="\\";
			str+=str2;
			build_ziplist_rec(str);

			continue;
		}

		str2.lower();
		int p=str2.find(".zip");
		if (p!=-1)
		{
			str+=str2;
			a.add(str);
			
			unzFile file;
			unz_file_info info;
			flyString name;
			int ok;

			file=unzOpen(str);
			ok=unzGoToFirstFile(file);
			while(ok==UNZ_OK)
			{
				char tmp[255];
				unzGetCurrentFileInfo(file,&info,tmp,255,0,0,0,0);

				name=tmp;
				while((p=name.find('/'))!=-1) name.set_char(p,'\\');

				a.add(name);
				
				ok=unzGoToNextFile(file);
			}
			unzClose(file);

			ziplist->add(a);
		}

	} while(FindNextFile(hFindFile, &FindFileData));
}

float flyFile::get_float()
{
	while(pos<len && !((buf[pos]>=48 && buf[pos]<=57) || buf[pos]=='.' || buf[pos]=='-'))
		pos++;

	if(pos==len) return 0;

	int i=pos;
	do i++;
	while(i<len && ((buf[i]>=48 && buf[i]<=57) || buf[i]=='.'));

	flyString str((char *)buf,pos,i-pos);
	pos=i;

	return (float)atof(str);
}

int flyFile::get_int()
{
	while(pos<len && !((buf[pos]>=48 && buf[pos]<=57) || buf[pos]=='-'))
		pos++;

	if(pos==len) return 0;

	int i=pos;
	do i++;
	while(i<len && buf[i]>=48 && buf[i]<=57);
		
	flyString str((char *)buf,pos,i-pos);
	pos=i;

	return atoi(str);
}

void flyFile::seek(int offset)
{
	pos=pos+offset;
	if (pos<0)
		pos=0;
	if (pos>=len)
		pos=len-1;
}

void flyFile::unzip(const char *localfile)
{
	FILE *fp;
	unzFile file;
	unz_file_info info;
	static char str[512],name[512];
	int ok;

	file=unzOpen(localfile);
	ok=unzGoToFirstFile(file);
	while(ok==UNZ_OK)
	{
		unzGetCurrentFileInfo(file,&info,name,255,0,0,0,0);
		
		strlwr(name);
		if (!strncmp(name,"data/",5))
		{
			strcpy(str,g_flyengine->flysdkdatapath);
			strcat(str,&name[5]);
		}
		else
		if (!strncmp(name,"plugin/",7))
		{
			strcpy(str,g_flyengine->flysdkpluginpath);
			strcat(str,&name[7]);
		}
		else
			continue;

		char *c1=str,*c2;
		while( c1 )
		{
			c2=strchr(c1,'/');
			if (c2==0)
				c2=strchr(c1,'\\');
			if (c2) 
			{			
				*c2=0;
				_mkdir(str);
				*c2='\\';
				c2++;
			}
			c1=c2;
		};

		fp=fopen(str,"wb");
		if (fp)
			{
			char *data=new char[info.uncompressed_size];
			unzOpenCurrentFile(file);
			unzReadCurrentFile(file,data,info.uncompressed_size);
			unzCloseCurrentFile(file);
			fwrite(data,1,info.uncompressed_size,fp);
			delete[] data;
			fclose(fp);
			}

		ok=unzGoToNextFile(file);
	}
	unzClose(file);
}

