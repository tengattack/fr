
#ifndef _TA_BASE_LOGS_H_
#define _TA_BASE_LOGS_H_
#pragma once

#include <windows.h>
#include <string>

namespace tengattack {
	namespace base {
		class Logs{
		public:
			Logs();
			~Logs();

			bool Open(LPCSTR path);
			bool Open(LPCWSTR path);
			bool Write(LPCSTR data);
			bool Write(LPCWSTR data);

		protected:

			bool preWrite();
			bool aftWrite();

			HANDLE m_hFile;
			bool m_bOpen;

			CRITICAL_SECTION m_csLock;

			std::wstring m_logspath;
		};
	};
};

#endif