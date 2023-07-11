
#include <windows.h>

#include <base/rand_util.h>

#include "strconv.h"
#include "Urlcode.h"

#include "string_easy_conv.h"

#ifdef NO_UTF8

char* utf8ToUrlFormat(const char *str, bool bIsPost)
{
	std::string strUrlf;
	char *UrlUtf8 = NULL;
	char data[4];
	data[3] = 0;

	/*int ii = 0;
	UrlUtf8[0] = 0;*/
	int len = strlen(str);
	for (int i = 0; i < len; i++)
	{
		//printf("%x ",(BYTE)str[i]);
		//英文字符不转换
		if ((BYTE)str[i] == '%')	//换行符
		{
			strUrlf.append("%25");
			//lstrcatA(UrlUtf8, "%26nbsp;");
		}
		else if ((BYTE)str[i] == ' ' && bIsPost)	//换行符
		{
			strUrlf.append("%26nbsp;");
			//lstrcatA(UrlUtf8, "%26nbsp;");
		}
		else if ((BYTE)str[i] == '\n' && bIsPost)	//换行符
		{
			strUrlf.append("<br>");
			//lstrcatA(UrlUtf8, "<br>");
		}
		else if ((BYTE)str[i] == '<' && bIsPost)
		{
			strUrlf.append("%26lt;");
			//lstrcatA(UrlUtf8, "%26lt;");
		}
		else if ((BYTE)str[i] == '>' && bIsPost)
		{
			strUrlf.append("%26gt;");
			//lstrcatA(UrlUtf8, "%26gt;");
		}
		else if ((((BYTE)str[i]) >= 0x20 && ((BYTE)str[i]) <= 0x25)
				 || (((BYTE)str[i]) >= 0x27 && ((BYTE)str[i]) <= 0x3B)
				 || (((BYTE)str[i]) == 0x3D)
				 || (((BYTE)str[i]) >= 0x3F && ((BYTE)str[i]) <= 0x7E))	//标准ASCII字符	
		{
			/*ii = lstrlenA(UrlUtf8);
			UrlUtf8[ii] = (BYTE)str[i];
			UrlUtf8[ii + 1] = 0;*/
			strUrlf.append(1, str[i]);
		}
		else
		{
			wsprintfA(data,"%%%02X",(BYTE)(str[i]));
			strUrlf.append(data);
			//lstrcatA(UrlUtf8, data);
		}

	}
	
	
	UrlUtf8 = _strdup(strUrlf.c_str());
	//strUrlf.clear();
	//delete[] UrlUtf8;
	return UrlUtf8;
}

char* MbToUrlFormat(const char *str, bool bIsPost)
{
	char *utf8 = MbToUtf8String(str);
  char *Urlf = NULL;
  if (utf8) {
    Urlf = utf8ToUrlFormat(utf8, bIsPost);
    free(utf8);
  }
	return Urlf;
}

char* MbToUtf8Url(const char *str)
{
	char *utf8 = MbToUtf8String(str);
  char *Urlf = NULL; //AllToUrlFormat(utf8);
  if (utf8) {
    lo_UrlEncodeA(&Urlf, utf8, lstrlenA(utf8));
    free(utf8);
  }
	return Urlf;
}

#endif

char* AllToUrlFormat(const char *str)
{
	char *urlfmt = NULL;
	lo_UrlEncodeA(&urlfmt, str, strlen(str));
	return urlfmt;
}

char* urlToAnsii(const char *str)
{
	char *destr = NULL;
	lo_UrlDecodeA(&destr, str, strlen(str));
	return destr;
}

char* WcToUtf8String(const wchar_t *str)
{
	char *utf8str = NULL;
	lo_W2Utf8(&utf8str, str);
	return utf8str;
}

wchar_t* utf8ToWcString(const char *str)
{
	wchar_t *wcstr = NULL;
	lo_Utf82W(&wcstr, str);
	return wcstr;
}

char* utf8ToMbString(const char *str)
{
	char *mbstr = NULL;
	lo_Utf82C(&mbstr, str);
	return mbstr;
}

char* MbToUtf8String(const char *str)
{
	char *utf8str = NULL;
	lo_C2Utf8(&utf8str, str);
	return utf8str;
}

char* WcToMb(const wchar_t *str)
{
	char *mbstr = NULL;
	lo_W2C(&mbstr, str);
	return mbstr;
}

wchar_t* MbToWc(const char *str)
{
	wchar_t *wcstr = NULL;
	lo_C2W(&wcstr, str);
	return wcstr;
}

void CodeFormat(const char *lspzFormat, char *szdata)
{
	int iLen = lstrlenA(lspzFormat);
	//char szdata[128];
	int idata = 0;
	for (int i = 0; i < iLen; i++)
	{
		if (lspzFormat[i] < 0)
		{
			//中文
			szdata[idata] = lspzFormat[i];
			szdata[idata + 1] = lspzFormat[i + 1];
			i++;
			idata += 2;
			continue;
		}
		if (lspzFormat[i] == '%')
		{
			if (i + 1 >= iLen)
			{
				break;
			}else{
				i++;
				switch (lspzFormat[i])
				{
				case 'd':
					szdata[idata] = base::RandInt('0', '9');	// 0x30 + rand1() % 10;
					break;
				case 'E':
					szdata[idata] = base::RandInt('A', 'Z');	//0x41 + rand1() % 26;
					break;
				case 'e':
					szdata[idata] = base::RandInt('a', 'z');	//0x61 + rand1() % 26;
					break;
				case 's':
					{
						wchar_t wcsText[2];
						wcsText[0] = base::RandInt(0x4E00, 0x9FA5);	//0x4E00 + rand1() % 0x51C0;
						wcsText[1] = 0;
						char *szText = WcToMb(wcsText);
						lstrcpyA(&szdata[idata], szText);
						idata += lstrlenA(szText) - 1;
						free(szText);
					}
					break;
				default:
					szdata[idata] = lspzFormat[i];
				}
			}
		}else{
			szdata[idata] = lspzFormat[i];
		}
		idata++;
	}
	szdata[idata] = 0;
}

wchar_t* Big5ToGb2312(const wchar_t *src)
{
	int len = lstrlenW(src);
	wchar_t *gb = (wchar_t *)malloc((len + 2) * sizeof(wchar_t));
	gb[0] = 0;
	LCMapStringW(0x0804,LCMAP_SIMPLIFIED_CHINESE, src, -1, gb, len + 1);
	//gb[len] = 0;
	return gb; 
}

wchar_t* Gb2312ToBig5(const wchar_t *src)
{
	int len = lstrlenW(src);
	wchar_t *gb = (wchar_t *)malloc((len + 2) * sizeof(wchar_t));
	gb[0] = 0;
	LCMapStringW(0x0804,LCMAP_TRADITIONAL_CHINESE, src, -1, gb, len + 1);
	//gb[len] = 0;
	return gb; 
}