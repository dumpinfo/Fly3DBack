#include "../Fly3D.h"

flyString::~flyString()
{ delete[] buf; }

flyString::flyString() : 
	size(FLY_STR_MINSIZE),len(0)
{ 
	buf=new char[size];
	buf[0]=0; 
}

flyString::flyString(const char* in) :
	len(strlen(in))
{
	size=FLY_STR_MINSIZE; 
	while( size<len+1 ) size<<=1;

	buf=new char[size];
	strcpy(buf,in);
}

flyString::flyString(const flyString& in) :
	len(in.len),size(in.size)
{
	buf=new char[size];
	strcpy(buf,(const char *)in);
}

flyString::flyString(const char* in,int p,int n) :
	len(n)
{
	size=FLY_STR_MINSIZE; 
	while( size<len+1 ) size<<=1;

	buf=new char[size];
	memcpy(buf,&in[p],len);
	buf[len]=0;
}

void flyString::operator=(const char* in)
{
	len=strlen(in);
	if(size<len+1)
	{
		while( size<len+1 ) size<<=1;

		delete[] buf;
		buf=new char[size];
	}
	strcpy(buf,in);
}

void flyString::copy(const char* in,int p,int n)
{
	len=n;
	if (size<len+1)
	{
		while( size<len+1 ) size<<=1;

		char* temp=new char[size];
		strncpy(temp,&in[p],len);
		delete[] buf;
		buf=temp;
	}
	else
		strncpy(buf,&in[p],len);
	buf[len]=0;
}

void flyString::format(const char *fmt, ...)
{
	static char ach[2048];
	
	va_list va;
	va_start( va, fmt );
	vsprintf( ach, fmt, va );
	va_end( va );

	operator=(ach);
}

flyString flyString::left(int n) const
{ 
	return flyString(buf,0,n); 
}

flyString flyString::right(int n) const
{ 
	return flyString(n<=len?&buf[len-n]:&buf[0]); 
}

flyString flyString::mid(int p,int n) const
{ 
	return flyString(buf,p,n); 
}

void flyString::operator=(const flyString& in)
{ 
	operator=((const char *)in); 
}

void flyString::operator+=(const char* in)
{
	len+=strlen(in);
	if(size<len+1)
	{
		while( size<len+1 ) size<<=1;

		char* temp=new char[size];
		strcpy(temp,buf);
		strcat(temp,in);
		delete[] buf;
		buf=temp;
	}
	else
		strcat(buf,in);
}

flyString flyString ::operator+(const flyString& in)
{ 
	return operator+((const char *)in); 
}

void flyString::operator+=(const flyString& in)
{ 
	operator+=((const char *)in); 
}

flyString flyString::operator+(const char* in)
{
	flyString str(buf);
	str+=in;
	return str;
}
