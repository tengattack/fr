
#ifndef _TA_JSON_CONFIG_H_
#define _TA_JSON_CONFIG_H_ 1
#pragma once

#include <base/basictypes.h>
#include <base/string/string16.h>

#include <string>

class Value;
class DictionaryValue;

namespace base {

	bool ReadJsonFile(LPCTSTR filepath, ::Value **val);

	class CJSONConfig {
	public:
		CJSONConfig();
		~CJSONConfig();

		void SetPath(LPCTSTR path_);
		bool Save();

		bool SetString(const std::string& path,
							const string16& in_value);
		bool SetString(const std::string& path,
							const std::string& in_value);
		bool SetBoolean(const std::string& path, bool in_value);
		bool SetInteger(const std::string& path, int in_value);

		bool GetString(const std::string& path,
							string16* out_value, LPCWSTR default_value);
		bool GetString(const std::string& path,
							std::string* out_value, LPCSTR default_value);
		bool GetBoolean(const std::string& path, bool* out_value, bool default_value);
		bool GetInteger(const std::string& path, int* out_value, int default_value);

		DictionaryValue* m_value;

	protected:
#ifdef UNICODE
    std::wstring m_path;
#else
    std::string m_path;
#endif
		bool m_changed;
		//base::CFile m_file;
	};
}

#endif