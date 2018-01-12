
#ifndef _TA_BASE_PROPERTIES_H_
#define _TA_BASE_PROPERTIES_H_ 1
#pragma once

#include <string>
#include <map>

#include <windows.h>

namespace base {
	typedef std::map<std::string, std::string> PropertiesMap;

	class Properties{
	public:
		Properties();
		virtual ~Properties();

		bool Has(LPCSTR name);
		void Put(LPCSTR name, LPCSTR data);
		void Get(LPCSTR name, std::string& data);
		void Get(int i, std::string& data);
		std::string& Get(LPCSTR name);

		void BuildProperties(LPCSTR separator, std::string& properties);

		unsigned long Size();

		inline PropertiesMap& GetMap()
		{
			return m_pro;
		}

	protected:
		PropertiesMap m_pro;
	};
};

#endif