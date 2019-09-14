/*! \file */

#pragma warning( disable : 4251 )
//! Dynamic array template class
/*!
	This template class implements an array allocated and expanded in execution time.
	It auto-manages its memory space and supplies methods for adding, removing and 
	indexing items.
*/  
template <class T> 
class flyArray
{
public:
	T *buf;		//!< generic buffer
	int num;	//!< number of entries
	
	int arraysize;	//!< array size in bytes

	//! Default constructor
	flyArray() : 
		arraysize(0), buf(0), num(0)
	{ }

	//! Copy-constructor
	flyArray(const flyArray<T>& in) :
		arraysize(0), buf(0), num(0)
	{ 
		reserve(in.num);
		for( int i=0;i<in.num;i++ )
			add(in[i]);
	}

	//! Default destructor
	virtual ~flyArray()
	{ delete[] buf; }

	//! Copy-constructor
	void operator=(const flyArray<T>& in)
	{ 
		reserve(in.num);
		clear();
		for( int i=0;i<in.num;i++ )
			add(in[i]);
	}

	//! Adds another array to the end of current array
	virtual void operator+=(flyArray<T>& in)
	{
		if (num+in.num>arraysize)
		{
			arraysize=num+in.num;
			T *tmp=new T[arraysize];
			for( int i=0;i<num;i++ )
				tmp[i]=buf[i];
			delete[] buf;
			buf=tmp;
		}
		for( int i=0;i<in.num;i++ )
			buf[i+num]=in[i];
		num+=in.num;
	}

	//! Reserve the required amount of space for the array
	virtual void reserve(int n)
	{
		if (arraysize<n)
		{
			arraysize=n;
			T *tmp=new T[arraysize];
			for( int i=0;i<num;i++ )
				tmp[i]=buf[i];
			delete[] buf;
			buf=tmp;
		}
	}

	//! Free all the space
	virtual void free()
	{
		delete[] buf;
		buf=0;
		num=0;
		arraysize=0;
	}

	//! Clear the whole array, but keep the memory space
	inline void clear()
	{
		num=0;
	}

	//! Add a new element to the end of the array, automatically allocating more space, if needed
	virtual void add(const T elem)
	{
		if (num==arraysize)
		{
			if (arraysize==0)
				arraysize=4;
			else
				arraysize+=(arraysize*3)>>1;
			T *tmp=new T[arraysize];
			for( int i=0;i<num;i++ )
				tmp[i]=buf[i];
			delete[] buf;
			buf=tmp;
		}
		buf[num++]=elem;
	}

	//! Remove the element in the given position
	inline void remove(int i)
	{
		if (i<num)
		{
			int j;
			for (j = i; j < num-1; j++) 
				buf[j] = buf[j+1];
			num--;
		}
	}

	//! Remove 'n' elements in the given position
	inline void remove(int i,int n)
	{
		if (i+n<=num)
		{
			int j;
			for (j = i; j < num-n; j++) 
				buf[j] = buf[j+n];
			num-=n;
		}
	}

	//! Indexing operator returing const
	inline const T& operator[](int i) const 
	{ return(buf[i]); }

	//! Indexing operator
	inline T& operator[](int i) 
	{ return buf[i]; }
};
