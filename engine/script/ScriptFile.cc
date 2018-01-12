
#include "stdafx.h"

#ifdef SCRIPT_FILE_HAVE_DOWNLOAD
#include <curlhelper.h>
#include <net/http/TANetBase.h>
#endif

#include "ScriptFile.h"
#include <windows.h>

CScriptFile::CScriptFile()
{
}

bool CScriptFile::Open(LPCWSTR path, LPCWSTR mode)
{
	Close();

	if (!path || !mode) return false;
	int len = lstrlenW(mode);
	if (len < 1) return false;

    DWORD dwMode = 0;

	bool append_ = false;

	switch (mode[0])
	{
	case 'r':
		dwMode = base::kFileRead;
		break;
	case 'w':
		dwMode = base::kFileWrite;
		break;
	case 'a':
		dwMode = base::kFileWrite;
		append_ = true;
		break;
	default:
		return false;
	}

	if (len >= 2)
	{
		switch (mode[len-1])
		{
		case '+':
			dwMode &= base::kFileCreate;
			break;
		default:
			return false;
		}
	}

	bool open_ = OpenW(dwMode, path);
	if (open_ && append_) {
		SetPointer(0, FILE_END);
	}

	return open_;
}

bool CScriptFile::ReadEx(unsigned char** data, int length, bool appendnull)
{
	if (IsOpen() && length > 0 && data)
	{
		unsigned char* _data = (unsigned char*)malloc(length + (appendnull ? 3 : 0));
		if (_data)
		{
			bool ret = Read(_data, length);
			if (ret)
			{
				if (appendnull) 
				{
					_data[length] = 0;
					_data[length+1] = 0;
					_data[length+2] = 0;
				}
				*data = _data;
			} else {
				free(_data);
			}
			return ret;
		}
	}
	return false;
}

int CScriptFile::HexToData(LPCSTR hexdata, unsigned char** data)
{
	int hexlen = lstrlenA(hexdata);
	if (hexlen % 2 != 0) return 0;

	int datalen = hexlen / 2;
	unsigned char* _data = (unsigned char*)malloc(datalen);
	if (_data)
	{
		char hexcode[3];
		hexcode[2] = 0;
		for (int i = 0; i < datalen; i++)
		{
			hexcode[0] = (unsigned char)(hexdata[i*2]);
			hexcode[1] = (unsigned char)(hexdata[i*2+1]);
			_data[i] = (unsigned char)strtoul(hexcode, NULL, 16);
		}

		*data = _data;
		return datalen;
	} else {
		return 0;
	}
}

int CScriptFile::DataToHex(unsigned char* data, int length, char** hexdata)
{
	int hexlen = length * 2;
	char* _hexdata = (char*)malloc(hexlen + 1);
	if (_hexdata)
	{
		char hexcode[3];
		for (int i = 0; i < length; i++)
		{
			wsprintfA(hexcode, "%02X", (unsigned char)data[i]);
			memcpy(_hexdata + i * 2, hexcode, 2);
		}

		_hexdata[length] = 0;
		*hexdata = _hexdata;
		return hexlen;
	} else {
		return 0;
	}
}

#ifdef SCRIPT_FILE_HAVE_DOWNLOAD
bool CScriptFile::DwonloadFrom(LPCSTR url)
{
	if (IsOpen()) {
		CTANetBase net_base;
		CBuffer buf;
		if (net_base.CurlRequestGet(url, buf) == TA_NET_OK) {
			return Write(buf.GetBuffer(), buf.GetBufferLen());
		}
	}
	return false;
}
#endif