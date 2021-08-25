
#include "file.h"

namespace base{

CFile::CFile()
	: m_hFile(INVALID_HANDLE_VALUE)
	, m_mode(kFileUnknow)
	//, m_path(L"")
	, m_opened(false)
{
}

CFile::CFile(unsigned long mode, LPCTSTR lpszPath)
	: m_hFile(INVALID_HANDLE_VALUE)
	, m_mode(mode)
	//, m_path(lpszPath ? lpszPath : L"")
	, m_opened(false)
{
	Open(mode, lpszPath);
}

CFile::~CFile()
{
	Close();
}

uint32 CFile::GetFileSize()
{
	return ::GetFileSize(m_hFile, NULL);
}

uint64 CFile::GetFileSize64()
{
	uint64 filesize = -1;
	if (!::GetFileSizeEx(m_hFile, (PLARGE_INTEGER)&filesize))
	{
		filesize = -1;
	}
	return filesize;
}

void CFile::Close()
{
	if (m_opened)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		m_opened = false;
	}
}

bool CFile::Read(unsigned char* data, unsigned long len)
{
	DWORD dwRead = 0;
	if (ReadFile(m_hFile, data, len, &dwRead, NULL))
		return true;
	else
		return false;
}

bool CFile::Write(unsigned char* data, unsigned long len)
{
	DWORD dwWrite = 0;
	if (WriteFile(m_hFile, data, len, &dwWrite, NULL))
		return true;
	else
		return false;
}

bool CFile::Flush()
{
	if (FlushFileBuffers(m_hFile))
		return true;
	else
		return false;
}

bool CFile::Delete(LPCWSTR lpszPath) 
{ 
	m_deleted = DeleteFile(lpszPath);
	return m_deleted;
}

bool CFile::OpenW(unsigned long mode, LPCWSTR lpszPath)
{
	//close prev file
	Close();

	DWORD dwDesiredAccess = 0;
	DWORD dwShareMode = 0;
	DWORD dwCreationDisposition = 0;

	if (mode & kFileCreate)
	{
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
		dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
		dwCreationDisposition = CREATE_ALWAYS;
	}
	if (mode & kFileRead)
	{
		dwDesiredAccess |= GENERIC_READ;
		dwShareMode |= FILE_SHARE_READ;
		dwCreationDisposition = OPEN_EXISTING;
	}
	if (mode & kFileWrite)
	{
		dwDesiredAccess |= GENERIC_WRITE;
		dwShareMode |= FILE_SHARE_WRITE;
		dwCreationDisposition = OPEN_EXISTING;
	}

	m_hFile = CreateFileW(lpszPath,
				dwDesiredAccess,
				dwShareMode, 
				NULL,
				dwCreationDisposition,
				FILE_ATTRIBUTE_NORMAL, 
				NULL);
	m_opened = (m_hFile != INVALID_HANDLE_VALUE);
	return m_opened;
}

bool CFile::OpenA(unsigned long mode, LPCSTR lpszPath)
{
	//close prev file
	Close();

	DWORD dwDesiredAccess = 0;
	DWORD dwShareMode = 0;
	DWORD dwCreationDisposition = 0;

	if (mode & kFileCreate)
	{
		dwDesiredAccess = GENERIC_ALL;
		dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
		dwCreationDisposition = CREATE_ALWAYS;
	}
	if (mode & kFileRead)
	{
		dwDesiredAccess |= GENERIC_READ;
		//dwShareMode |= FILE_SHARE_READ;
		dwCreationDisposition = OPEN_EXISTING;
	}
	if (mode & kFileWrite)
	{
		dwDesiredAccess |= GENERIC_WRITE;
		dwShareMode |= FILE_SHARE_WRITE;
		dwCreationDisposition = OPEN_EXISTING;
	}

	m_hFile = CreateFileA(lpszPath,
				dwDesiredAccess,
				dwShareMode, 
				NULL,
				dwCreationDisposition,
				FILE_ATTRIBUTE_NORMAL, 
				NULL);

	m_opened = (m_hFile != INVALID_HANDLE_VALUE);
	return m_opened;
}

bool CFile::OpenLog(unsigned long mode, LPCWSTR lpszPath)
{
  // close prev file
  Close();

  DWORD dwDesiredAccess = 0;
  DWORD dwShareMode = 0;
  DWORD dwCreationDisposition = 0;

  if (mode & kFileCreate) {
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    dwCreationDisposition = CREATE_ALWAYS;
  }
  if (mode & kFileRead) {
    dwDesiredAccess |= GENERIC_READ;
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    dwCreationDisposition = OPEN_EXISTING;
  }


  m_hFile = CreateFileW(lpszPath, dwDesiredAccess, dwShareMode, NULL,
                        dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
  m_opened = (m_hFile != INVALID_HANDLE_VALUE);
  return m_opened;
}

bool CFile::SetPointer(uint32 offset, unsigned long dwMoveMethod)
{
	LARGE_INTEGER li = {0};
	li.LowPart = offset;
	return (::SetFilePointerEx(m_hFile, li, NULL, dwMoveMethod) != HFILE_ERROR);
}

bool CFile::SetPointer(uint64 offset, unsigned long dwMoveMethod)
{
	LARGE_INTEGER li;
	memcpy(&li, &offset, sizeof(uint64));
	return (::SetFilePointerEx(m_hFile, li, NULL, dwMoveMethod) != HFILE_ERROR);
}

bool CFile::IsOpen()
{
	return m_opened;
}

};