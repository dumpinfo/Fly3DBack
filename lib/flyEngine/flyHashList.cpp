#include "../Fly3D.h"

flyHashList::flyHashList() :
	count(0)
{
}

flyHashList::~flyHashList()
{
	reset();
}

void flyHashList::reset()
{
	int i;
	for( i=0;i<m_hash.num;i++ )
		delete m_list[i];
	m_hash.free();
	m_list.free();
	m_last.free();
	count=0;
}

void flyHashList::add(flyBaseObject *obj)
{
	int i;
	for( i=0;i<m_hash.num;i++ )
		if (m_hash[i]==obj->type)
			break;
	
	if (i==m_hash.num)
	{
		m_hash.add(obj->type);
		m_list.add(0);
		m_last.add(0);
	}

	if (m_last[i]==0)
	{
		m_list[i]=obj;
		m_last[i]=obj;
	}
	else
	{
		m_last[i]->next_obj=obj;
		m_last[i]=obj;
	}

	count++;
	obj->next_obj=0;
}

void flyHashList::remove(flyBaseObject *obj)
{
	int i;
	for( i=0;i<m_hash.num;i++ )
		if (m_hash[i]==obj->type)
			break;
	if (i==m_hash.num)
		return;

	flyBaseObject *o=m_list[i],*op=0;
	while(o)
	{
		if (o==obj)
		{
			if (op==0)
				m_list[i]=o->next_obj;
			else
				op->next_obj=o->next_obj;
			if (o==m_last[i])
				m_last[i]=op;
			break;
		}
		else
		{
			op=o;
			o=o->next_obj;
		}
	}

	count--;
	obj->next_obj=0;
}

flyBaseObject *flyHashList::get_object_name(const char *name,int type) const
{
	if (name[0]==0)
		return 0;

	flyBaseObject *obj=0;
	int i;

	if (type)
	{
		for( i=0;i<m_hash.num;i++ )
			if (m_hash[i]==type)
				break;
		if (i==m_hash.num)
			return 0;

		obj=m_list[i];
		while(obj)
		{
			if (obj->name.compare_nocase(name)==0)
				break;
			obj=obj->next_obj;
		}
	}
	else
	{
		for( i=0;i<m_hash.num;i++ )
		{
			obj=m_list[i];
			while(obj)
			{
				if (obj->name.compare_nocase(name)==0)
					break;
				obj=obj->next_obj;
			}
			if (obj)
				break;
		}
	}

	return obj;
}

flyBaseObject *flyHashList::get_object_longname(const char *long_name,int type) const
{
	if (long_name[0]==0)
		return 0;

	flyBaseObject *obj=0;
	int i;

	if (type)
	{
		for( i=0;i<m_hash.num;i++ )
			if (m_hash[i]==type)
				break;
		if (i==m_hash.num)
			return 0;

		obj=m_list[i];
		while(obj)
		{
			if (obj->long_name.compare_nocase(long_name)==0)
				break;
			obj=obj->next_obj;
		}
	}
	else
	{
		for( i=0;i<m_hash.num;i++ )
		{
			obj=m_list[i];
			while(obj)
			{
				if (obj->long_name.compare_nocase(long_name)==0)
					break;
				obj=obj->next_obj;
			}
			if (obj)
				break;
		}
	}

	return obj;
}

flyBaseObject *flyHashList::get_next_object(const flyBaseObject *o,int type) const
{
	if (o)
		if (o->next_obj)
			return o->next_obj;
		else
			if (type)
				return 0;
			else
			{
				int i;
				for( i=0;i<m_hash.num;i++ )
					if (m_hash[i]==o->type)
						break;
				for( i++;i<m_hash.num;i++ )
					if (m_list[i])
						break;
				if (i>=m_hash.num)
					return 0;
				return m_list[i];
			}
	else
		if (type==0)
			if (m_hash.num)
				return m_list[0];
			else
				return 0;
		else
		{
			int i;
			for( i=0;i<m_hash.num;i++ )
				if (m_hash[i]==type)
					break;
			if (i==m_hash.num)
				return 0;
			return m_list[i];
		}
}
