#include "../Fly3D.h"

template <class T>
flyArray<T>::~flyArray()
{ 
	delete[] buf; 
}
