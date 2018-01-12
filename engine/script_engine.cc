
#include "stdafx.h"

#include <iostream>
#include <string>
#include <v8.h>

#include "script_engine.h"
#include "script_func.h"

#include <base/file/file.h>
#include <base/file/filedata.h>

#include <base/string/stringprintf.h>

#include <common/strconv.h>

#include <filecommon/file_path.h>

#pragma comment(lib, "v8.lib")

namespace v8Global{
	//Handle<ObjectTemplate> *m_pGlobal = NULL;
};

typedef struct _TA_MAPFILE_DATA {
	/*BYTE *bData;
	DWORD dwSize;
	HANDLE hFileMap;*/
	std::string *text;
	_SCRIPT_CALLBACK script_callback;
	void *script_callback_user;
} TA_MAPFILE_DATA;

CTAScriptEngine::CTAScriptEngine(void)
{
	m_szScriptFolder[0] = 0;
}


CTAScriptEngine::~CTAScriptEngine(void)
{
	//HandleScope handle_scope;
}

void CTAScriptEngine::Init(LPCTSTR lpszScriptFolder, LPCTSTR base_path)
{
	int nLen = lstrlen(lpszScriptFolder);
	if (nLen > 0)
	{
		lstrcpy(m_szScriptFolder, lpszScriptFolder);
		::CreateDirectory(m_szScriptFolder, NULL);
		if (m_szScriptFolder[nLen - 1] != '\\')
		{
			m_szScriptFolder[nLen] = '\\';
			m_szScriptFolder[nLen+1] = 0;
		}
	}

	/*if (base_path && lstrlen(base_path) > 0) {
		v8func::current_path = base_path;
	}*/
#ifdef UNICODE
	SetScriptCurrentPath(m_szScriptFolder);
#else
	wchar_t *wpath = NULL;
	lo_C2W(&wpath, m_szScriptFolder);
	SetScriptCurrentPath(wpath);
	if (wpath) free(wpath);
#endif

	//::CreateNamedPipeA
	v8::V8::Initialize();
}

int CTAScriptEngine::ListScript(ScriptList& list, LPCTSTR ext)
{
	int count = 0;
	HANDLE lf;
	TCHAR findpath[MAX_PATH];
	wsprintf(findpath, _T("%s\\*"), m_szScriptFolder);

	WIN32_FIND_DATA file;
	if((lf = FindFirstFile(findpath, &file))!=INVALID_HANDLE_VALUE)//_findfirst返回的是long型; long __cdecl _findfirst(const char *, struct _finddata_t *)
	{
		//std::cout << "\n文件列表:\n";
		//int __cdecl _findnext(long, struct _finddata_t *);如果找到下个文件的名字成功的话就返回0,否则返回-1
		do {
			if (file.cFileName[0] == L'.')
				continue;
			if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//wsprintf(findpath, L"%s\\%s", m_szScriptFolder, file.cFileName);
				//不包含子文件夹
			} else {
				bool enable_ = true;
				if (ext) {
					enable_ = false;
					LPCTSTR fileext = GetFileNameExt(file.cFileName);
					if (fileext && lstrcmpi(fileext, ext) == 0) {
						enable_ = true;
					}
				}
				if (enable_) {
					SCRIPT_INFO si;
#ifdef UNICODE
					si.filename = file.cFileName;
#else
					wchar_t *wname = NULL;
					lo_C2W(&wname, file.cFileName);
					if (wname) {
						si.filename = wname;
						free(wname);
					}
#endif
					GetScriptInfo(file.cFileName, si.info);
					list.push_back(si);

					count++;
				}
			}
		} while(FindNextFile(lf, &file));
	}
	FindClose(lf);
	return count;
}

/*
int CTAScriptEngine::ShowScriptsToCombo(CComboBox *pComboBox)
{
	if (!pComboBox) return -1;

	int OriCount = pComboBox->GetCount();
	int i, iCount = 0;
	if (OriCount > 0)
	{
		for (i = 0;i<OriCount;i++)
		{
			pComboBox->DeleteString(0);
		}
		pComboBox->Clear();
	}

    WIN32_FIND_DATA fd;
    ZeroMemory(&fd, sizeof(WIN32_FIND_DATA));
 
    HANDLE hSearch;
    TCHAR filePathName[MAX_PATH];
	//TCHAR tmpPath[MAX_PATH];
    ZeroMemory(filePathName, MAX_PATH);
	//ZeroMemory(tmpPath, MAX_PATH);

    lstrcpy(filePathName, m_szScriptFolder);
    lstrcat(filePathName, _T("*"));
 
    hSearch = FindFirstFile(filePathName, &fd);
 
	if (hSearch == INVALID_HANDLE_VALUE) return 0;
    //Is directory
    if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
		if (lstrcmpi(fd.cFileName + lstrlen(fd.cFileName) - 3, _T(".js")) == 0)
		{
			iCount++;
			pComboBox->AddString(fd.cFileName);
		}
    }
 
    while(FindNextFile(hSearch, &fd))
    {
		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (lstrcmpi(fd.cFileName + lstrlen(fd.cFileName) - 3, _T(".js")) == 0)
			{
				iCount++;
				pComboBox->AddString(fd.cFileName);
			}
		}
    }
 
    FindClose(hSearch);

	if (iCount > 0)
	{
		pComboBox->SetCurSel(0);
	}
	return iCount;
}
*/

bool CTAScriptEngine::GetScriptInfo(LPCTSTR szFile, std::wstring& info)
{
	TCHAR szFilePath[MAX_PATH];
	lstrcpy(szFilePath, m_szScriptFolder);
	lstrcat(szFilePath, szFile);

	/*char *szData = NULL;
	DWORD dwSize = 0;

	HANDLE hFileMap = ReadAllFromFile(szFilePath, (BYTE **)&szData, &dwSize);
	if (hFileMap == INVALID_HANDLE_VALUE || dwSize <= 2)
	{
		if (hFileMap != INVALID_HANDLE_VALUE)
		{
			::UnmapViewOfFile(szData);
			::CloseHandle(hFileMap);
		}
		return _T("错误的脚本文件！");
	}*/

	base::CFile file;

	if (file.Open(base::kFileRead, szFilePath)) {
		base::CFileData fd;
		if (fd.Read(file)) {
			std::wstring wscript;
			fd.ToText(wscript);

			int nLen = 0;
			if ((nLen = wscript.length()) > 0) {
				const wchar_t *lpUnicodeData = wscript.c_str();
				const wchar_t *szScriptDes = lpUnicodeData + 6;
				const wchar_t *rn = wcschr(szScriptDes, '\n');
				
				if (rn) {
					if (*(rn - 1) == '\r') rn--;
					nLen = rn - szScriptDes;
				}
				if (nLen > 0) {
					if (wcsncmp(lpUnicodeData, L"//<-- ", 6) == 0) {
						info = std::wstring(szScriptDes, nLen);
						return true;
					}
				}
			}
		}
		file.Close();
	}

	return false;

	/*
	//源码字符串
	char *szInfoData = NULL;
	if (((BYTE *)szData)[0] == 0xff && ((BYTE *)szData)[1] == 0xfe)
	{
		//Unicode
		wchar_t *lpUnicodeData = (wchar_t *)(szData + 2);

		if (wcsncmp(lpUnicodeData, L"//<-- ", 6) != 0)
		{
			goto noinfo_l;
		} else {
			wchar_t *szScriptDes = lpUnicodeData + 6;
			wchar_t *rn = wcschr(szScriptDes, '\n');
			if (!rn) goto noinfo_l;
			if (*(rn - 1) == '\r') rn--;

			int nLen = rn - szScriptDes;
			if (nLen <= 0) goto noinfo_l;

			wchar_t *szUniInfoData = (wchar_t *)malloc((nLen + 1) * sizeof(wchar_t));
			memcpy(szUniInfoData, szScriptDes, nLen * sizeof(wchar_t));
			szUniInfoData[nLen] = 0;
			szInfoData = ::WcToMb(szUniInfoData);
			free(szUniInfoData);
		}
	} else {
		if (strncmp((LPCSTR)szData, "//<-- ", 6) != 0) {
			return _T("该脚本文件无描述信息！");
		}

		char *szScriptDes = szData + 6;
		char *rn = strchr(szScriptDes, '\n');
		if (!rn) goto noinfo_l;
		if (*(rn -1) == '\r') rn--;

		int nLen = rn - szScriptDes;
		if (nLen <= 0) goto noinfo_l;

		szInfoData = (char *)malloc(nLen + 1);
		memcpy(szInfoData, szScriptDes, nLen);
		szInfoData[nLen] = 0;
		
	}

	CString szInfo(szInfoData);
	free(szInfoData);

	return szInfo;
	*/
}

HANDLE CTAScriptEngine::Run(LPCTSTR szFile, DWORD *pThreadId)
{
	return RunEx(szFile, false, pThreadId);
}

HANDLE CTAScriptEngine::RunEx(LPCTSTR szFile, _SCRIPT_CALLBACK callback, void *user, bool bwait, DWORD *pThreadId)
{
	TCHAR sPath[MAX_PATH];
	lstrcpy(sPath, m_szScriptFolder);
	lstrcat(sPath, szFile);
	
	TA_MAPFILE_DATA *pMapData = (TA_MAPFILE_DATA *)malloc(sizeof(TA_MAPFILE_DATA));

	/*pMapData->hFileMap = ReadAllFromFile(sPath, &pMapData->bData, &pMapData->dwSize);
	if (pMapData->hFileMap == INVALID_HANDLE_VALUE || pMapData->dwSize <= 2)
	{
		if (pMapData->hFileMap != INVALID_HANDLE_VALUE)
		{
			::UnmapViewOfFile(pMapData->bData);
			::CloseHandle(pMapData->hFileMap);
		}
		free(pMapData);
		return NULL;
	}*/
	base::CFile file;
	if (file.Open(base::kFileRead, sPath)) {
		base::CFileData fd;
		if (fd.Read(file)) {
			pMapData->text = new std::string;
			fd.ToUtf8Text(*pMapData->text);

			pMapData->script_callback = callback;
			pMapData->script_callback_user = user;

			HANDLE hThread = ::CreateThread(NULL, NULL, ScriptShellProc, (LPVOID)pMapData, CREATE_SUSPENDED, pThreadId);
			if (hThread == NULL) {
				free(pMapData);
			} else {
				if (bwait) {
					::ResumeThread(hThread);
					::WaitForSingleObject(hThread, INFINITE);
					//CloseHandle(hThread);
				}
			}

			return hThread;
		}
	}

	return NULL;
}

DWORD WINAPI CTAScriptEngine::ScriptShellProc(LPVOID lParam)
{
	TA_MAPFILE_DATA *pMapData = (TA_MAPFILE_DATA *)lParam;

	try {
		v8::Locker Locker;
		HandleScope handle_scope;	// 管理Handle，在它生命周期内的Handle都归它管

		Handle<ObjectTemplate> global_templ = ObjectTemplate::New();

		v8func::AddMyFunc(global_templ);
		v8class::AddMyClass(global_templ);

		_SCRIPT_CALLBACK script_callback = pMapData->script_callback;
		void *callback_user = pMapData->script_callback_user;

		if (script_callback) {
			script_callback(callback_user, global_templ, NULL, NULL, kSCSInit);
		}

		Persistent<Context> m_pContext = Context::New(NULL, global_templ);	//建立上下文
		Context::Scope m_pContext_scope = Context::Scope(m_pContext);	//管理上下文

		if (script_callback) {
			script_callback(callback_user, global_templ, &m_pContext->Global(), NULL, kSCSContext);
		}

		Handle<String> source_obj;

		//源码字符串
		/*if (pMapData->bData[0] == 0xff && pMapData->bData[1] == 0xfe)
		{
			//Unicode
			source_obj = String::New((const uint16_t *)(pMapData->bData + 2), (pMapData->dwSize - 2) / sizeof(uint16_t));
		} else {
			int iLen = MultiByteToWideChar(CP_ACP, 0, (char *)pMapData->bData, pMapData->dwSize, NULL, NULL);
			if (iLen > 0)
			{
				wchar_t *wcuff = (wchar_t *)malloc((iLen + 1) * sizeof(wchar_t));
				MultiByteToWideChar(CP_ACP, 0, (char *)pMapData->bData, pMapData->dwSize, wcuff, iLen);
				wcuff[iLen] = 0;

				source_obj = String::New((const uint16_t *)wcuff, iLen);

				free(wcuff);
			} else {

				source_obj = String::New((const char *)pMapData->bData, pMapData->dwSize);
			}
		}
		
	
		::UnmapViewOfFile(pMapData->bData);
		::CloseHandle(pMapData->hFileMap);*/
		source_obj = String::New(pMapData->text->c_str());
		delete pMapData->text;

		free(pMapData);

		v8::TryCatch trycatch;
		Handle<Script> script = Script::Compile(source_obj);	//编译
	
		if (!script.IsEmpty())
		{
			if (script_callback) {
				script_callback(callback_user, global_templ, &m_pContext->Global(), &script, kSCSStart);
			}
			script->Run();//运行
		}

		if (trycatch.HasCaught())
		{
#ifdef _CONSOLE
			std::wstring szInfo;
			Handle<String> e = trycatch.Exception()->ToString();
			Handle<String> sl = trycatch.Message()->GetSourceLine();

			wchar_t *szException = NULL;
			wchar_t *szSourceLine = NULL;
			lo_V8S2W(&szException, e);
			lo_V8S2W(&szSourceLine, sl);

			base::SStringPrintf(&szInfo,
				_T("脚本[%s]\r\n(%d) %s\r\n发现错误，错误描述：%s\r\n"), SCRIPT_TITLEW, 
					trycatch.Message()->GetLineNumber(),
					szSourceLine,
					szException);

			free(szException);
			free(szSourceLine);

			wprintf(L"%s\n", szInfo.c_str());
#endif
			if (script_callback) {
				script_callback(callback_user, global_templ, &m_pContext->Global(), &trycatch, kSCSError);
			}
		}

		if (script_callback) {
			script_callback(callback_user, global_templ, &m_pContext->Global(), &script, kSCSFinish);
		}
		
		m_pContext.Dispose();

	} catch (...) {
		;
	}
	return 0;
}