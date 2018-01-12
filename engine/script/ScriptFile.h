

#ifndef _TA_SCRIPT_FILE_
#define _TA_SCRIPT_FILE_

#pragma once

#include <string>
#include <base/file/file.h>
#include <base/file/filedata.h>
#include "ScriptBase.h"

class CScriptFile : public CScriptBase, public base::CFile {
public:
	CScriptFile();

	bool Open(LPCWSTR path, LPCWSTR mode);

	bool ReadEx(unsigned char** data, int length, bool appendnull = false);

#ifdef SCRIPT_FILE_HAVE_DOWNLOAD
	bool DwonloadFrom(LPCSTR url);
#endif

	static int HexToData(LPCSTR hexdata, unsigned char** data);
	static int DataToHex(unsigned char* data, int length, char** hexdata);

protected:
	//std::wstring m_path_;
};

#endif //_TA_SCRIPT_FILE_