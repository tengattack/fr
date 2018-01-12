

#include "stdafx.h"
#include "file_patch.h"
#include "file_control.h"
#include <tcmalloc.h>

#if 0
void ShowError()
{
	TCHAR* buffer = NULL;
	DWORD dwError = ::GetLastError();
	::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				dwError,
				0,
				( LPTSTR )&buffer,
				0,
				NULL );
	_tprintf(_T("´íÎó´úÂë£º0x%02x\n´íÎóÃèÊö£º%s\n"),dwError,buffer);
	LocalFree( buffer );
}
#endif

int file_patch(LPCWSTR patchfile, LPCWSTR writefile, FP_CALLBACK callback, void *user)
{
	BYTE *lpByte = NULL;
	DWORD dwSize = 0;

	TCHAR szPatcher[MAX_PATH];
	GetModuleFileName(NULL, szPatcher, MAX_PATH);
	for (int i = lstrlen(szPatcher) - 1; i > 0; i--)
	{
		if (szPatcher[i] == '\\')
		{
			szPatcher[i + 1] = 0;
			break;
		}
	}

	HANDLE hFileMap = ReadAllFromFile(patchfile, &lpByte, &dwSize);
	if (hFileMap == INVALID_HANDLE_VALUE)
	{
		return FP_ERROR_OPEN_PATCH;
	}

	HANDLE hFileOut = CreateFile(writefile,
				    GENERIC_ALL,
					FILE_SHARE_WRITE, 
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL, 
					NULL);
	if (hFileOut == INVALID_HANDLE_VALUE) {
		UnmapViewOfFile(lpByte);
		CloseHandle(hFileMap);
		return FP_ERROR_OPEN_WRITE;
	}

	
	unsigned __int64 uSize = 0;
	GetFileSizeEx(hFileOut, (PLARGE_INTEGER)&uSize);

	if (memcmp(&uSize, lpByte, 8) != 0) {
		UnmapViewOfFile(lpByte);
		CloseHandle(hFileMap);
		CloseHandle(hFileOut);
		return FP_ERROR_CHECK;
	}

	int iSucceed = 0;
	DWORD dwCount = *(DWORD *)(lpByte + 8);
	DWORD iPos = 8 + 4,dwLenth,dwWrites = 0;
	unsigned __int64 *puOffset;
	OVERLAPPED ol = {0};

	for (DWORD i = 0;i < dwCount && iPos < dwSize;i++)
	{
		puOffset = (unsigned __int64 *)(lpByte + iPos);
		iPos += 8;

		/*_tprintf(_T("[0x%08x%08x] "),
				((LARGE_INTEGER*)puOffset)->HighPart, 
				((LARGE_INTEGER*)puOffset)->LowPart);*/

		dwLenth = *(DWORD *)(lpByte + iPos);
		//_tprintf(_T("%d\t"),dwLenth);
		iPos += 4;

		ol.OffsetHigh = ((LARGE_INTEGER*)puOffset)->HighPart;
		ol.Offset = ((LARGE_INTEGER*)puOffset)->LowPart;
		ol.InternalHigh = dwLenth;
		if (WriteFile(hFileOut, lpByte + iPos, dwLenth, &dwWrites, &ol)) {
			iSucceed++;
		}

		iPos += dwLenth;
		if (callback) callback(user, i + 1, dwCount);
	}

	UnmapViewOfFile(lpByte);
	CloseHandle(hFileMap);
	CloseHandle(hFileOut);

	if (iSucceed == dwCount)
	{
		return FP_SUCCESS;
	} else if (iSucceed == 0){
		return FP_ERROR_ALL;
	} else {	
		return FP_ERROR_PART;
	}
}

