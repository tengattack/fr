
#include "stdafx.h"

#include "file_control.h"

BOOL WriteToFile(HANDLE hFile, BYTE* data, DWORD dwSize, DWORD offset)
{
	DWORD dwWrite;

	if (::SetFilePointer(hFile, offset, NULL, FILE_BEGIN) != HFILE_ERROR)
	{
		return::WriteFile(hFile, data, dwSize, &dwWrite, NULL);
	} else {
		return FALSE;
	}
}

HANDLE ReadAllFromFileA(LPCSTR lpFileName,BYTE** data,unsigned long *Size)
{
     HANDLE hFile = CreateFileA(lpFileName,
				    GENERIC_READ,
					FILE_SHARE_READ, 
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_SEQUENTIAL_SCAN, 
					NULL);

	 if (hFile == INVALID_HANDLE_VALUE)   
	 {   
		//_tprintf(_T("[Open file] Open file error!\n"));
		return INVALID_HANDLE_VALUE;   
	 }
	 unsigned long FileSize=0;
	 FileSize=GetFileSize(hFile,NULL);
	 HANDLE hFileMapping = CreateFileMapping(hFile,NULL,PAGE_READONLY, 
		                   0, 0, NULL);
	 CloseHandle(hFile);

     *Size = FileSize;

	 *data = (BYTE*)MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,0);   

     return(hFileMapping);
}

HANDLE ReadAllFromFileW(LPCWSTR lpFileName,BYTE** data,unsigned long *Size)
{
     HANDLE hFile = CreateFileW(lpFileName,
				    GENERIC_READ,
					FILE_SHARE_READ, 
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_SEQUENTIAL_SCAN, 
					NULL);

	 if (hFile == INVALID_HANDLE_VALUE)   
	 {   
		//_tprintf(_T("[Open file] Open file error!\n"));
		return INVALID_HANDLE_VALUE;   
	 }
	 unsigned long FileSize=0;
	 FileSize=GetFileSize(hFile,NULL);
	 HANDLE hFileMapping = CreateFileMapping(hFile,NULL,PAGE_READONLY, 
		                   0, 0, NULL);
	 CloseHandle(hFile);

     *Size = FileSize;

	 *data = (BYTE*)MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,0);   

     return(hFileMapping);
}

HANDLE StartWriteToFileA(LPCSTR lpFileName, DWORD dwFileAttrib)
{
	if (dwFileAttrib == 0)
		dwFileAttrib = FILE_ATTRIBUTE_NORMAL;
	HANDLE hFile = CreateFileA(lpFileName,
				    GENERIC_ALL,
					FILE_SHARE_WRITE, 
					NULL,
					CREATE_ALWAYS,
					dwFileAttrib, 
					NULL);
	return hFile;
}

HANDLE StartWriteToFileW(LPCWSTR lpFileName, DWORD dwFileAttrib)
{
	if (dwFileAttrib == 0)
		dwFileAttrib = FILE_ATTRIBUTE_NORMAL;
	HANDLE hFile = CreateFileW(lpFileName,
				    GENERIC_ALL,
					FILE_SHARE_WRITE, 
					NULL,
					CREATE_ALWAYS,
					dwFileAttrib, 
					NULL);
	return hFile;
}

/*bool GetDialog(bool bSave, wchar_t *sFileName, wchar_t *sFilter, wchar_t *sRet, wchar_t *InitialDir)
{

    BOOL rtn;
	OPENFILENAME File={0};
    File.lStructSize = sizeof(OPENFILENAME);
    File.hInstance = 0;
	File.lpstrFile = new wchar_t[256];
	memcpy(File.lpstrFile,sFileName,lstrlen(sFileName) + 1);
    //File.lpstrFile = sFileName ;
    File.nMaxFile = 255;
    File.lpstrFileTitle = new wchar_t[256];
    File.nMaxFileTitle = 255;
    File.lpstrInitialDir = InitialDir;
    File.lpstrFilter = sFilter;
	File.lpstrTitle = (bSave ? _T("保存文件") : _T("打开文件"));
    if (!bSave)
	{
        File.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	}
	else
	{
        //File.lpstrDefExt = GetFix(sFileName)
        File.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	}

	rtn = GetSaveFileName(&File);
	delete[] File.lpstrFileTitle;
    if (rtn)
	{
		memcpy(sRet,File.lpstrFile,MAX_PATH * sizeof(wchar_t));
        //sRet = File.lpstrFile;
		delete[] File.lpstrFile;
        return true;
	}
	else
	{
		delete[] File.lpstrFile;
        return false;
	}

}*/
