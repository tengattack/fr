

#ifndef _TA_SCRIPT_ENGINE_SQLITE_
#define _TA_SCRIPT_ENGINE_SQLITE_

#pragma once

#include <base/basictypes.h>

#include <string>
#include <vector>
#include <map>
#include "ScriptBase.h"

#include <db/sqlite.h>

class CScriptSqlite : public CSqlite, public CScriptBase {
public:
	CScriptSqlite();
	~CScriptSqlite();
};

#endif //_TA_SCRIPT_SQLITE_