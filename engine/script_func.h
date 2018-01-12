
#ifndef _TA_COMMON_SCRIPT_FUNC_H_
#define _TA_COMMON_SCRIPT_FUNC_H_

#include <common/strconv.h>
#include <filecommon/file_path.h>

#if defined(SNOW)
#include "interface/work/script.h"
#include "dialog/dialog_input.h"
#elif defined(_SNOW_ZERO)
#include "SnowScriptInterface.h"
#else
#define SCRIPT_TITLEW L"script"
#endif

#include "script/ScriptThreadPool.h"

//class
#include "script/ScriptTAV8Http.h"
#include "script/ScriptFileDialog.h"
#include "script/ScriptFile.h"
#include "script/ScriptSqlite.h"

#define SCRIPT_ENGINE_VERSION L"0.8"

using namespace v8;

CScriptThreadPool script_tpool;

int lo_V8S2W(wchar_t **pout, Handle<String> v8string)
{
	return lo_Utf82W(pout, *(String::Utf8Value(v8string)));
}

int lo_V8S2C(char **pout, Handle<String> v8string)
{
	return lo_Utf82C(pout, *(String::Utf8Value(v8string)));
}

int lo_V8S2Utf8(char **pout, Handle<String> v8string)
{
	int clen = v8string->Utf8Length();
	char *utf8text = (char *)malloc(clen + 1);
	if (utf8text == NULL) {
		return 0;
	}
	memcpy(utf8text, *String::Utf8Value(v8string), clen);
	utf8text[clen] = 0;
	*pout = utf8text;
	return clen;
}

#ifdef SCRIPT_HAVE_EXIT
extern bool BeforeExit();
#endif

namespace v8func {

#include "script_thread.h"
std::wstring current_path;

Handle<String> MbToV8String(LPCSTR lpszString)
{
	wchar_t *wtext = NULL;
	lo_C2W(&wtext, lpszString);
	Handle<String> ret_str = String::New((uint16_t *)wtext);
	free(wtext);

	return ret_str;
}

Handle<Value> CurrentScript(const Arguments& args)
{
	HandleScope handle_scope;

	return String::New((uint16_t *)(LPCWSTR)SCRIPT_TITLEW);
}

#ifdef _SNOW
Handle<Value> GlobalPrint(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	
	int c = args.Length();
	if (c > 0)
	{
		bool bNoTime = false;
		WORD nColor = CONSOLE_TEXT_COLOR_WHITE;

		if (c >= 3)	bNoTime = args[2]->BooleanValue();
		if (c >= 2)	nColor = (WORD)args[1]->Uint32Value();

		Handle<String> s = args[0]->ToString();

		wchar_t *szTmp = NULL;
		lo_V8S2W(&szTmp, s);

		if (bNoTime) {
			rPrintW((szTmp ? szTmp : L""), nColor);
		} else {
			cPrintW((szTmp ? szTmp : L""), nColor);
		}

		free(szTmp);
	}
	return Undefined();
}
#endif

Handle<Value> Print(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	
	int c = args.Length();
	if (c > 0)
	{
#ifdef _CONSOLE
		for (int i = 0; i < c; i++) {
			Handle<String> s = args[i]->ToString();
			wchar_t *wtext = NULL;
			lo_V8S2W(&wtext, s);
			wprintf(L"%s ", wtext);
			free(wtext);
		}
		wprintf(L"\n");
#else
		std::wstring str;
		for (int i = 0; i < c; i++) {

			Handle<String> s = args[i]->ToString();

			if (i > 0) {
				str += L" ";
			}

			wchar_t *wtext = NULL;
			lo_V8S2W(&wtext, s);
			
			str += wtext;

			free(wtext);
		}

		S_FUNC(Print)(str.c_str());
#endif
	}
	return Undefined();
}

Handle<Value> Alert(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	
	if (args.Length() > 0)
	{
		//if (args[0]->IsString())
		{
			Handle<String> s = args[0]->ToString();
			MessageBoxW(NULL, (LPCWSTR)*(String::Value(s)), SCRIPT_TITLEW, MB_OK | MB_ICONINFORMATION);
		}
	}
	//std::cout << *(String::AsciiValue(s)) << std::endl;
	return Undefined();
}

Handle<Value> Confirm(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	
	if (args.Length() > 0)
	{
		bool msgYes = false;
		//if (args[0]->IsString())
		{
			Handle<String> s = args[0]->ToString();
			msgYes = (MessageBoxW(NULL, (LPCWSTR)*(String::Value(s)), SCRIPT_TITLEW, MB_YESNO | MB_ICONQUESTION) == IDYES);
		}
		if (msgYes)
		{
			return ::v8::True();
		} else {
			return ::v8::False();
		}
	}
	//std::cout << *(String::AsciiValue(s)) << std::endl;
	return Undefined();
}

#ifdef SCRIPT_HAVE_EXIT
Handle<Value> MyExit(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	
	uint32 exit_code = 0;
	if (args.Length() > 0) {
		exit_code = args[0]->Uint32Value();
	}
	if (BeforeExit()) {
		ExitProcess(exit_code);
	}

	return Undefined();
}
#endif

Handle<Value> MySleep(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	
	if (args.Length() > 0)
	{
		Sleep(args[0]->Uint32Value());
	}
	//std::cout << *(String::AsciiValue(s)) << std::endl;
	return Undefined();
}

Handle<Value> ScriptInclude(const Arguments& args)
{
	HandleScope handle_scope;

	bool bret = false;
	if (args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();

		wchar_t *filepath = NULL;
		lo_V8S2W(&filepath, s);

		if (filepath) {

			std::wstring path = current_path;
			path += filepath;
			free(filepath);

			base::CFile file;
			if (file.OpenW(base::kFileRead, path.c_str())) {
				base::CFileData fd;
				if (fd.Read(file)) {

					std::string utf8text;
					fd.ToUtf8Text(utf8text);

					Handle<Script> script = Script::Compile(String::New(utf8text.c_str()));	//编译

					if (!script.IsEmpty()) {
						return script->Run();//运行
					}
				}
			}
		}
	}
	
	return False();
}

Handle<Value> Prompt(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	
	int c = args.Length();
	wchar_t *szInfo = NULL, *szDef = NULL;

	if (c > 0) {
		Handle<String> s = args[0]->ToString();
		lo_V8S2W(&szInfo, s);
	}
	if (c > 1) {
		Handle<String> s = args[1]->ToString();
		lo_V8S2W(&szDef, s);
	}

#ifdef _CONSOLE
	wprintf(L"%s (%s):\n", szInfo, szDef);

	if (szInfo) free(szInfo);

	wchar_t wtext[256];
	wscanf(L"%s", wtext);

	if (lstrlenW(wtext) <= 0) {
		lstrcpy(wtext, szDef);
	}

	if (szDef) free(szDef);

	return String::New((uint16_t *)wtext); //MbToV8String(idlg.m_str);
#else
	#if defined(SNOW)
		view::frame::CSnowDialogInput dlgInput(NULL, SCRIPT_TITLEW,
			(szInfo ? szInfo : L""),
			szDef);

		if (szInfo) free(szInfo);
		if (szDef) free(szDef);

		if (dlgInput.doInput()) {
			return String::New((uint16_t *)dlgInput.getInput().c_str());
		} else {
			return v8::Null();
		}
	#elif defined(_SNOW_ZERO)
		#ifdef UNICODE
			CTaInputDlg idlg((LPCTSTR)SCRIPT_TITLE.GetString(), (szInfo ? szInfo : ""), szDef);
		#else
			char *a_szInfo = NULL, *a_szDef = NULL;
			if (szInfo) lo_W2C(&a_szInfo, szInfo);
			if (szDef) lo_W2C(&a_szDef, szDef);

			CTaInputDlg idlg((LPCTSTR)SCRIPT_TITLE.GetString(), (a_szInfo ? a_szInfo : ""), a_szDef);
		#endif

		idlg.DoModal();

		if (szInfo) free(szInfo);
		if (szDef) free(szDef);

		#ifdef UNICODE
			return String::New((uint16_t *)idlg.m_str.GetString());
		#else
			if (a_szInfo) free(a_szInfo);
			if (a_szDef) free(a_szDef);

			return MbToV8String(idlg.m_str);
		#endif

	#else
		return v8::Null();
	#endif
#endif
}

Handle<Value> GetCurrentPath(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	//当前脚本引擎版本

	if (current_path.length() <= 0) {
		wchar_t path[MAX_PATH];
		path[0] = 0;
		if (GetModuleFileNameW(NULL, path, MAX_PATH)) {
			for (int i = lstrlenW(path) - 1; i > 0; i--) {
				if (path[i] == '\\' || path[i] == '/') {
					path[i + 1] = 0;
					current_path = path;
					break;
				}
			}
		}
	}

	Handle<String> ret_str = String::New((uint16_t *)current_path.c_str());
	return ret_str;
}

/*Handle<Value> Exec(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	
	if (args.Length() > 0)
	{
		Handle<String> v8s_command = args[1]->ToString();
		char *utf8command = NULL;
		lo_V8S2Utf8(&utf8command, v8s_command);

		free(utf8command);
	}
	//std::cout << *(String::AsciiValue(s)) << std::endl;
	return Undefined();
}*/

Handle<Value> ScriptEngineVersion(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)
	//当前脚本引擎版本
	Handle<String> ret_str = String::New((uint16_t *)SCRIPT_ENGINE_VERSION);
	return ret_str;
}

#ifdef _DEBUG
Handle<Value> Test(const Arguments& args)
{
	HandleScope handle_scope; //管理这里的Handle(函数退出时释放循环里的Handle<String> s)

	Handle<Object> ret_str = Object::New();
	//ret_str->SetInternalFieldCount(1) ; 
	Handle<String> hcookie = String::New("cookie");

	ret_str->Set(hcookie, hcookie, (PropertyAttribute)(v8::ReadOnly));

	return ret_str;
}
#endif


//Process Control
#if defined(HAVE_PROCESS_CONTROL)
Handle<Value> ProcessSetRange(const Arguments& args)
{
	HandleScope handle_scope;

	if (args.Length() > 1) {
		int low = args[0]->Int32Value();
		int up = args[1]->Int32Value();

		if (up >= low) {
			SP_FUNC(SetRange)(low, up);
		}
	}

	return Undefined();
}

Handle<Value> ProcessSetPos(const Arguments& args)
{
	HandleScope handle_scope;

	if (args.Length() > 0) {
		SP_FUNC(SetPos)(args[0]->Int32Value());
	}

	return Undefined();
}

Handle<Value> ProcessGetPos(const Arguments& args)
{
	HandleScope handle_scope;

	return v8::Int32::New(SP_FUNC(GetPos()));
}

Handle<Value> ProcessStepIt(const Arguments& args)
{
	HandleScope handle_scope;

	SP_FUNC(StepIt)();

	return Undefined();
}

Handle<Value> ProcessSetStep(const Arguments& args)
{
	HandleScope handle_scope;

	if (args.Length() > 0) {
		SP_FUNC(SetStep)(args[0]->Int32Value());
	}

	return Undefined();
}
#endif


void AddMyFunc(Handle<ObjectTemplate> global) //加入自定义函数
{
	global->Set(String::New("ScriptEngineVersion"), FunctionTemplate::New(ScriptEngineVersion));

	global->Set(String::New("GlobalPrint"), FunctionTemplate::New(Print));	//都用print
	global->Set(String::New("GetCurrentScript"), FunctionTemplate::New(CurrentScript));
	global->Set(String::New("GetCurrentPath"), FunctionTemplate::New(GetCurrentPath));

#ifdef _DEBUG
	global->Set(String::New("Test"), FunctionTemplate::New(Test));
#endif

	AddThreadFunc(global);

	//Add Process Control
#if defined(HAVE_PROCESS_CONTROL)
	global->Set(String::New("ProcessSetRange"), FunctionTemplate::New(ProcessSetRange));
	global->Set(String::New("ProcessSetPos"), FunctionTemplate::New(ProcessSetPos));
	global->Set(String::New("ProcessSetStep"), FunctionTemplate::New(ProcessSetStep));
	global->Set(String::New("ProcessGetPos"), FunctionTemplate::New(ProcessGetPos));
	global->Set(String::New("ProcessStepIt"), FunctionTemplate::New(ProcessStepIt));
#endif

	global->Set(String::New("print"), FunctionTemplate::New(Print));
	global->Set(String::New("alert"), FunctionTemplate::New(Alert));
	global->Set(String::New("conf->rm"), FunctionTemplate::New(Confirm));
	global->Set(String::New("prompt"), FunctionTemplate::New(Prompt));

	global->Set(String::New("sleep"), FunctionTemplate::New(MySleep));

	global->Set(String::New("include"), FunctionTemplate::New(ScriptInclude));
	//global->Set(String::New("exec"), FunctionTemplate::New(Exec));
	/*global->Set(String::New("callfunc"), FunctionTemplate::New(CallFunc));*/

#ifdef SCRIPT_HAVE_EXIT
	global->Set(String::New("exit"), FunctionTemplate::New(MyExit));
#endif
}

};

namespace v8class{
	#include "Script\ClsTAHttp.h"
	#include "Script\ClsFileDialog.h"
	#include "Script\ClsFile.h"
#ifdef USE_SQLITE
	#include "Script\ClsSqlite.h"
#endif

void AddMyClass(Handle<ObjectTemplate> global) //加入自定义函数
{
	AddTAHttp(global);
	AddClsFileDialog(global);
	AddClsFile(global);
#ifdef USE_SQLITE
	AddClsSqlite(global);
#endif
}
};

void SetScriptCurrentPath(LPCWSTR path)
{
	if (path) {
		v8func::current_path = path;
		if (!HaveRightSlashW(v8func::current_path.c_str())) {
			v8func::current_path += '\\';
		}
	} else {
		v8func::current_path.clear();
	}
}

#endif //_TA_SNOW_SCRIPT_FUNC_H_