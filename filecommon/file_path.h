
#ifndef _TA_FILE_COMMON_PATH_H_
#define _TA_FILE_COMMON_PATH_H_ 1
#pragma once

#include "file_path.h"
#include <windows.h>

LPCSTR GetFileNameA(LPCSTR path);
LPCWSTR GetFileNameW(LPCWSTR path);

LPCSTR GetFileNameExtA(LPCSTR path);
LPCWSTR GetFileNameExtW(LPCWSTR path);

LPCSTR GetFilePathA(char* path);
LPCWSTR GetFilePathW(wchar_t* path);

bool HaveRightSlashA(LPCSTR lpszPath);
bool HaveRightSlashW(LPCWSTR lpszPath);

BOOL DirectoryExists(LPCTSTR szPath);

#ifdef UNICODE
#define GetFileName GetFileNameW
#define GetFileNameExt GetFileNameExtW
#define GetFilePath GetFilePathW
#define HaveRightSlash HaveRightSlashW
#else
#define GetFileName GetFileNameA
#define GetFileNameExt GetFileNameExtA
#define GetFilePath GetFilePathA
#define HaveRightSlash HaveRightSlashA
#endif

bool CreateDir(LPCTSTR dir, bool every_ = false);

#endif