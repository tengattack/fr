
#ifndef _SNOW_CSTRING_UNTIL_H_
#define _SNOW_CSTRING_UNTIL_H_ 1

#include <windows.h>
#include <string>

namespace base {
class CStringA : public std::string{
public:
	CStringA();
	CStringA(const char* text);

	void Format(const char* format, ...);
	void AppendFormat(const char* format, ...);
	void Append(const char* text);

	int GetLength();
	const char* GetString();

	operator const char *()
	{
		return c_str();
	}
};

class CStringW : public std::wstring{
public:
	CStringW();
	CStringW(const wchar_t* text);

	void Format(const wchar_t* format, ...);
	void AppendFormat(const wchar_t* format, ...);
	void Append(const wchar_t* text);

	int GetLength();
	const wchar_t* GetString();

	operator const wchar_t *()
	{
		return c_str();
	}
};
};

#endif