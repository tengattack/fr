
#include "stdafx.h"
#include "cstring_util.h"
#include "stringprintf.h"

#ifdef CStringA
#undef CStringA
#endif

namespace base {

CStringA::CStringA()
{
}

CStringA::CStringA(LPCSTR text)
	: std::string(text)
{
}

void CStringA::Format(const char* format, ...)
{
	this->clear();

	va_list ap;
	va_start(ap, format);
	base::StringAppendV((std::string *)this, format, ap);
	va_end(ap);
}

void CStringA::Append(const char* text)
{
	this->append(text);
}

void CStringA::AppendFormat(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	base::StringAppendV((std::string *)this, format, ap);
	va_end(ap);
}

int CStringA::GetLength()
{
	return lstrlenA(c_str());
}

const char* CStringA::GetString()
{
	return c_str();
}


CStringW::CStringW()
{
}

CStringW::CStringW(LPCWSTR text)
	: std::wstring(text)
{
}

void CStringW::Format(const wchar_t* format, ...)
{
	this->clear();

	va_list ap;
	va_start(ap, format);
	base::StringAppendV((std::wstring *)this, format, ap);
	va_end(ap);
}

void CStringW::Append(const wchar_t* text)
{
	this->append(text);
}

void CStringW::AppendFormat(const wchar_t* format, ...)
{
	va_list ap;
	va_start(ap, format);
	base::StringAppendV((std::wstring *)this, format, ap);
	va_end(ap);
}

int CStringW::GetLength()
{
	return lstrlenW(c_str());
}

const wchar_t* CStringW::GetString()
{
	return c_str();
}
};