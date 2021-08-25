
#ifndef _SNOW_CUTE_BASE_FILE_H_
#define _SNOW_CUTE_BASE_FILE_H_ 1

#include "../common.h"
#include "../basictypes.h"

namespace base{
	enum FILE_MODE{
		kFileUnknow = 0,
		kFileCreate = 1,
		kFileRead = 2,
		kFileWrite = 4
	};
	class CFile{
	public:
		CFile();
		CFile(unsigned long mode, LPCTSTR lpszPath);
		~CFile();

		uint32 GetFileSize();
		uint64 GetFileSize64();

		inline bool Open(unsigned long mode, LPCTSTR lpszPath)
		{
#ifdef _UNICODE
			return OpenW(mode, lpszPath);
#else
			return OpenA(mode, lpszPath);
#endif
		}
		inline bool ShareOpen(unsigned long mode, LPCTSTR lpszPath) {
#ifdef _UNICODE
			return ShareOpenW(mode, lpszPath);
#else
			return ShareOpenA(mode, lpszPath);
#endif
		}
		static inline bool Delete(LPCTSTR lpszPath) {
#ifdef _UNICODE
			return DeleteW(lpszPath);
#else
			return DeleteA(lpszPath);
#endif
		}
		bool OpenW(unsigned long mode, LPCWSTR lpszPath);
		bool OpenA(unsigned long mode, LPCSTR lpszPath);
		bool ShareOpenW(unsigned long mode, LPCWSTR lpszPath);
		bool ShareOpenA(unsigned long mode, LPCSTR lpszPath);

		bool IsOpen();
		void Close();

		bool Read(unsigned char* data, unsigned long len);
		bool Write(unsigned char* data, unsigned long len);
		bool SetPointer(uint32 offset, unsigned long dwMoveMethod = FILE_BEGIN);
		bool SetPointer(uint64 offset, unsigned long dwMoveMethod = FILE_BEGIN);

		bool Flush();

		static bool DeleteW(LPCWSTR lpszPath);
		static bool DeleteA(LPCSTR lpszPath);

	protected:
		bool m_opened;
		HANDLE m_hFile;
		unsigned long m_mode;

		//std::wstring m_path;
	};
};

#endif