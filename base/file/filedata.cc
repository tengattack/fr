
#include "filedata.h"

namespace base{

CFileData::CFileData()
	: m_size(0)
	, m_data(NULL)
{
}

CFileData::~CFileData()
{
	Clear();
}

void CFileData::Clear()
{
	if (m_size > 0)
	{
		free(m_data);
		m_data = NULL;
		m_size = 0;
	}
}

bool CFileData::Read(CFile& file)
{
	Clear();

	m_size = file.GetFileSize();

	if (m_size > 0)
	{
		m_data = (unsigned char*)malloc(m_size);
		if (m_data)
		{
			if (file.Read(m_data, m_size))
			{
				return true;
			} else {
				Clear();
			}
		}
	} else if (m_size == 0) {
		return true;
	}

	m_size = 0;
	return false;
}

unsigned char* CFileData::GetData()
{
	return m_data;
}

TEXT_FORMAT CFileData::GetDataFormat()
{
	if (m_data)
	{
		if (m_size >= 2)
		{
			if (m_data[0] == 0xff && m_data[1] == 0xfe)
			{
				return kTFUnicode;
			}

			if (m_size >= 3)
			{
				if (m_data[0] == 0xef && m_data[1] == 0xbb && m_data[2] == 0xbf)
				{
					return kTFUtf8;
				}
			}
		}

		return kTFASCII;
	}

	return kTFUnknow;
}


void CFileData::ToText(std::wstring& text)
{
	if (m_data)
	{
		int len, textlen;
		switch (GetDataFormat())
		{
		case kTFASCII:
			textlen = m_size;
			len = MultiByteToWideChar(CP_ACP, 0, (char *)m_data, textlen, NULL, NULL);
			if (len > 0)
			{
				wchar_t *wcuff = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
				if (!wcuff) return;
				MultiByteToWideChar(CP_ACP, 0, (char *)m_data, textlen, wcuff, len);
				wcuff[len] = 0;

				text = wcuff;
				free(wcuff);
			}
			break;
		case kTFUnicode:
			text = std::wstring((wchar_t *)(m_data + 2), (m_size - 2) / sizeof(wchar_t));
			break;
		case kTFUtf8:
			textlen = (m_size - 3);
			len = MultiByteToWideChar(CP_UTF8, 0, (char *)(m_data + 3), textlen, NULL, 0);
			if (len > 0)
			{
				wchar_t *t_str = (wchar_t *)malloc((len+1) * sizeof(wchar_t));
				if (!t_str) return;
				MultiByteToWideChar(CP_UTF8, 0, (char *)(m_data + 3), textlen, t_str, len);
				t_str[len] = 0;

				text = t_str;
				free(t_str);
			}
			break;
		}
	}
}

void CFileData::ToText(std::string& text)
{
	if (m_data)
	{
		int len, textlen;
		switch (GetDataFormat())
		{
		case kTFASCII:
			text = std::string((char *)m_data, m_size);
			break;
		case kTFUnicode:
			textlen = (m_size - 2) / sizeof(wchar_t);
			len = WideCharToMultiByte(CP_ACP, 0, (wchar_t *)(m_data + 2), textlen, NULL, 0, NULL, NULL);
			if (len > 0)
			{
				char *mbuff = (char *)malloc(len + 1);
				if (!mbuff) return;
				WideCharToMultiByte(CP_ACP, 0, (wchar_t *)(m_data + 2), textlen, mbuff, len, NULL, NULL);
				mbuff[len] = 0;

				text = mbuff;
				free(mbuff);
			}
			break;
		case kTFUtf8:
			textlen = (m_size - 3);
			len = MultiByteToWideChar(CP_UTF8, 0, (char *)(m_data + 3), textlen, NULL, 0);
			if (len > 0)
			{
				wchar_t *t_str = (wchar_t *)malloc((len+1) * sizeof(wchar_t));
				if (!t_str) return;
				MultiByteToWideChar(CP_UTF8, 0, (char *)(m_data + 3), textlen, t_str, len);
				t_str[len] = 0;

				int buffLen = WideCharToMultiByte(CP_UTF8, 0, t_str, len, NULL, 0, 0, 0);
				if (buffLen)
				{
					char *mbuff = (char *)malloc(buffLen + 1);
					if (mbuff)
					{
						WideCharToMultiByte(CP_ACP, 0, t_str, len, mbuff, buffLen, 0, 0);
						mbuff[buffLen] = 0;

						text = mbuff;
						free(mbuff);
					}
				}

				free(t_str);
			}

			break;
		}
	}
}

void CFileData::ToUtf8Text(std::string& text)
{
	if (m_data)
	{
		int len, textlen;
		switch (GetDataFormat())
		{
		case kTFASCII:
			textlen = m_size;
			len = MultiByteToWideChar(CP_ACP, 0, (char *)(m_data), textlen, NULL, 0);
			if (len > 0)
			{
				wchar_t *t_str = (wchar_t *)malloc((len+1) * sizeof(wchar_t));
				if (!t_str) return;
				MultiByteToWideChar(CP_ACP, 0, (char *)(m_data), textlen, t_str, len);
				t_str[len] = 0;

				int buffLen = WideCharToMultiByte(CP_UTF8, 0, t_str, len, NULL, 0, 0, 0);
				if (buffLen)
				{
					char *mbuff = (char *)malloc(buffLen + 1);
					if (mbuff)
					{
						WideCharToMultiByte(CP_UTF8, 0, t_str, len, mbuff, buffLen, 0, 0);
						mbuff[buffLen] = 0;

						text = mbuff;
						free(mbuff);
					}
				}

				free(t_str);
			}
			break;
		case kTFUnicode:
			textlen = (m_size - 2) / sizeof(wchar_t);
			len = WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)(m_data + 2), textlen, NULL, 0, NULL, NULL);
			if (len > 0)
			{
				char *mbuff = (char *)malloc(len + 1);
				if (!mbuff) return;
				WideCharToMultiByte(CP_UTF8, 0, (wchar_t *)(m_data + 2), textlen, mbuff, len, NULL, NULL);
				mbuff[len] = 0;

				text = mbuff;
				free(mbuff);
			}
			break;
		case kTFUtf8:
			textlen = (m_size - 3);
			text = std::string((char *)(m_data + 3), textlen);
			break;
		}
	}
}

uint32 CFileData::GetSize()
{
	return m_size;
}


};