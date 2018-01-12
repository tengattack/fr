
#ifndef _TA_SCRIPT_ENGINE_H_
#define _TA_SCRIPT_ENGINE_H_ 1

#pragma once

#include <string>
#include <vector>
#include <winsock2.h>
#include <windows.h>

#include <v8.h>

#include "script/ScriptThreadPool.h"

extern CScriptThreadPool script_tpool;

enum ScriptCallbackStatus {
	kSCSInit = 0,
	kSCSStart,
	kSCSContext,
	kSCSFinish,
	kSCSError,
};

int lo_V8S2W(wchar_t **pout, v8::Handle<v8::String> v8string);
int lo_V8S2C(char **pout, v8::Handle<v8::String> v8string);
int lo_V8S2Utf8(char **pout, v8::Handle<v8::String> v8string);

typedef void (CALLBACK * _SCRIPT_CALLBACK)(void *user,
		v8::Handle<v8::ObjectTemplate> templ, v8::Handle<v8::Object> *obj,
		void *ptr, ScriptCallbackStatus status);

typedef struct _SCRIPT_INFO {
	std::wstring filename;
	std::wstring info;
} SCRIPT_INFO;

typedef std::vector<SCRIPT_INFO> ScriptList;

class CTAScriptEngine
{
public:
	CTAScriptEngine();
	virtual ~CTAScriptEngine();

	void Init(LPCTSTR lpszScriptFolder, LPCTSTR base_path);
	int ListScript(ScriptList& list, LPCTSTR ext = _T("js"));

	HANDLE Run(LPCTSTR szFile, DWORD *pThreadId);
	HANDLE RunEx(LPCTSTR szFile, _SCRIPT_CALLBACK callback = NULL, void *user = NULL, bool bwait = false, DWORD *pThreadId = NULL);

	//int ShowScriptsToCombo(CComboBox *pComboBox);

	bool GetScriptInfo(LPCTSTR szFile, std::wstring& info);

	static DWORD WINAPI ScriptShellProc(LPVOID lParam);

protected:
	TCHAR m_szScriptFolder[MAX_PATH];
};

#endif