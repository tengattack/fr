
#include "StdAfx.h"
#include "logs.h"
#include <string>
#include <stringfmt.h>

namespace tengattack {
	namespace base {
		Logs::Logs()
			: m_hFile(INVALID_HANDLE_VALUE)
			, m_bOpen(false)
		{
			InitializeCriticalSection(&m_csLock);
		}

		Logs::~Logs()
		{
			aftWrite();
		}

		bool Logs::preWrite()
		{
			m_hFile = CreateFileW(m_logspath.c_str(),
				    GENERIC_ALL,
					FILE_SHARE_READ | FILE_SHARE_WRITE, 
					NULL,
					OPEN_ALWAYS,
					NULL, 
					NULL);
			
			m_bOpen = (m_hFile != INVALID_HANDLE_VALUE);

			if (m_bOpen)
			{
				DWORD size = ::GetFileSize(m_hFile, NULL);
				if (size > 0 && size != 0xffffffff)
				{
					::SetFilePointer(m_hFile, 0, NULL, FILE_END);
				}
			}
			return m_bOpen;
		}

		bool Logs::aftWrite()
		{
			if (m_bOpen && m_hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_hFile);
			}

			m_bOpen = false;
			m_hFile = INVALID_HANDLE_VALUE;

			return true;
		}

		bool Logs::Open(LPCSTR path)
		{
			wchar_t *wpath = MbToWc((char *)path);
			return Open(wpath);
		}

		bool Logs::Open(LPCWSTR path)
		{
			m_logspath = path;
			if (!preWrite())
			{
				return false;
			}
			aftWrite();
			return true;
		}

		bool Logs::Write(LPCSTR data)
		{
			//if (!m_bOpen) return false;
			
			EnterCriticalSection(&m_csLock);
			if (!preWrite())
			{
				LeaveCriticalSection(&m_csLock);
				return false;
			}

			CStringA strOut;
			DWORD dwWrite;
			SYSTEMTIME st = {0};

			GetLocalTime(&st);
			strOut.Format("[%04d/%02d/%02d %02d:%02d:%02d] - %s\r\n",  
				st.wYear,st.wMonth,st.wDay,
				st.wHour,st.wMinute,st.wSecond, 
				data);

			BOOL b = ::WriteFile(m_hFile, (LPCSTR)strOut, strOut.GetLength(), &dwWrite, NULL);

			aftWrite();
			LeaveCriticalSection(&m_csLock);

			return (b != FALSE);
		}

		bool Logs::Write(LPCWSTR data)
		{
			//if (!m_bOpen) return false;
			char* mbdata = WcToMb((wchar_t *)data);
			if (mbdata)
			{
				bool b = Write(mbdata);
				free(mbdata);
				return b;
			} else {
				return false;
			}
		}
	
	};
};