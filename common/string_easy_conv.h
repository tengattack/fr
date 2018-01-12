
#ifndef _TA_STRING_FMT_H_
#define _TA_STRING_FMT_H_ 1

#include <common/strconv.h>

#ifdef NO_UTF8

char* MbToUtf8Url(const char *str);
char* MbToUrlFormat(const char *str, bool bIsPost);

char* utf8ToUrlFormat(const char *str, bool bIsPost);

#endif

char* AllToUrlFormat(const char *str);
char* urlToAnsii(const char *str);

char* WcToUtf8String(const wchar_t *str);
wchar_t* utf8ToWcString(const char *str);
char* utf8ToMbString(const char *str);
char* MbToUtf8String(const char *str);

char* WcToMb(const wchar_t *str);
wchar_t* MbToWc(const char *str);

//szdata需要先分配好空间
void CodeFormat(const char *lspzFormat, char *szdata);

wchar_t* Big5ToGb2312(const wchar_t *src);
wchar_t* Gb2312ToBig5(const wchar_t *src);

#endif