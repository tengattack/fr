
#ifndef _FILECONTROL_H_
#define _FILECONTROL_H_

#include <windows.h>
#include <tchar.h>

HANDLE ReadAllFromFileA(LPCSTR lpFileName,BYTE** data,unsigned long *Size);
HANDLE ReadAllFromFileW(LPCWSTR lpFileName,BYTE** data,unsigned long *Size);
HANDLE StartWriteToFileA(LPCSTR lpFileName, DWORD dwFileAttrib = 0);
HANDLE StartWriteToFileW(LPCWSTR lpFileName, DWORD dwFileAttrib = 0);
BOOL WriteToFile(HANDLE hFile, BYTE* data, DWORD dwSize, DWORD offset);

#ifdef _UNICODE
#define ReadAllFromFile ReadAllFromFileW
#define StartWriteToFile StartWriteToFileW
#else
#define ReadAllFromFile ReadAllFromFileA
#define StartWriteToFile StartWriteToFileA
#endif


#endif //#ifndef _FILECONTROL_H_