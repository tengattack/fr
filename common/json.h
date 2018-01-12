
#ifndef _TA_BASE_JSON_H_
#define _TA_BASE_JSON_H_
#pragma once

#include <string>
#include <map>
#include "properties.h"

namespace tengattack {
	namespace base {

		std::string& replace_all(std::string& str, const std::string& old_value, const std::string& new_value);

		class JSONReader{
		public:
			JSONReader();
			~JSONReader();

			bool Read(LPCSTR json);

			void Get(LPCSTR name, std::string& data);

			inline ::base::Properties& GetProperties()
			{
				return m_properties;
			}

		protected:
			::base::Properties m_properties;

			static void ProcessValue(LPCSTR value, int len, std::string& out);
		};
	};
};

#endif