
#include "properties.h"

namespace base {
Properties::Properties()
{
}
Properties::~Properties()
{
}

bool Properties::Has(LPCSTR name)
{
	if (name)
	{
		for (PropertiesMap::iterator it = m_pro.begin();
			 it != m_pro.end(); it++)
		{
			if (it->first == name)
			{
				return true;
				//break;
			}
		}
	}
	return false;
}

void Properties::Put(LPCSTR name, LPCSTR data)
{
	if (name && data)
		m_pro[name] = data;
}

void Properties::Get(LPCSTR name, std::string& data)
{
	if (name)
	{
		data = m_pro[name];
	}
}

void Properties::Get(int i, std::string& data)
{
	if (i < (int)m_pro.size() && i >= 0)
	{
		PropertiesMap::iterator it = m_pro.begin();
		for (int j = 0; j < i; j++)
			it++;
		data = it->second;
	}
}

std::string& Properties::Get(LPCSTR name)
{
	return m_pro[name];
}

void Properties::BuildProperties(LPCSTR separator, std::string& properties)
{
	//properties = "";
	for (PropertiesMap::iterator it = m_pro.begin();
		 it != m_pro.end(); it++)
	{
		properties += it->first;
		properties += "=";
		properties += it->second;
		if (separator) properties += separator;
	}
}

unsigned long Properties::Size()
{
	return m_pro.size();
}

};
