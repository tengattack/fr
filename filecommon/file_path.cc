
#include "stdafx.h"
#include "file_path.h"

#include <tchar.h>

BOOL DirectoryExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

LPCSTR GetFileNameA(LPCSTR path)
{
	for (int i = strlen(path) - 1; i > 0; i--) {
		if (path[i] == '\\' || path[i] == '/') {
			return path + i + 1;
		}
	}
	return path;
}

LPCWSTR GetFileNameW(LPCWSTR path)
{
	for (int i = lstrlenW(path) - 1; i > 0; i--) {
		if (path[i] == '\\' || path[i] == '/') {
			return path + i + 1;
		}
	}
	return path;
}

LPCSTR GetFileNameExtA(LPCSTR path)
{
	for (int i = strlen(path) - 1; i > 0; i--)
	{
		if (path[i] == '.') {
			return path + i + 1;
			break;
		} else if (path[i] == '\\' || path[i] == '/') {
			break;
		}
	}
	return NULL;
}

LPCWSTR GetFileNameExtW(LPCWSTR path)
{
	for (int i = lstrlenW(path) - 1; i > 0; i--)
	{
		if (path[i] == '.') {
			return path + i + 1;
			break;
		} else if (path[i] == '\\' || path[i] == '/') {
			break;
		}
	}
	return NULL;
}

LPCSTR GetFilePathA(char* path)
{
	for (int i = strlen(path) - 1; i > 0; i--) {
		if (path[i] == '\\' || path[i] == '/') {
			path[i + 1] = 0;
			break;
		}
	}
	return path;
}

LPCWSTR GetFilePathW(wchar_t* path)
{
	for (int i = lstrlenW(path) - 1; i > 0; i--) {
		if (path[i] == '\\' || path[i] == '/') {
			path[i + 1] = 0;
			break;
		}
	}
	return path;
}

bool HaveRightSlashA(LPCSTR lpszPath)
{
	long sLen = strlen(lpszPath);
    if (sLen == 0) {  //不需要\了
        return false;
	} else {
		char ch = lpszPath[sLen - 1];
		return (ch == '\\' || ch == '/');
	}
}

bool HaveRightSlashW(LPCWSTR lpszPath)
{
	long sLen = lstrlenW(lpszPath);
    if (sLen == 0) {  //不需要\了
        return false;
	} else {
		wchar_t ch = lpszPath[sLen - 1];
		return (ch == '\\' || ch == '/');
	}
}

bool CreateDir(LPCTSTR dir, bool every_)
{
	BOOL bret = FALSE;

	if (every_) {
		long len = lstrlen(dir);
		int dircount = 0;
		TCHAR tdirpath[MAX_PATH];
		
		for (int i = 0; i < len; i++) {
			if (dir[i] == '\\' || dir[i] == '/') {
				dircount++;
				if (dircount > 1) {

					memcpy(tdirpath, dir, (i * sizeof(TCHAR)));
					tdirpath[i] = 0;

					bret = ::CreateDirectory(tdirpath, NULL);
				}
			}
		}
	} else {
		bret = ::CreateDirectory(dir, NULL);
	}

	return static_cast<bool>(bret);
}