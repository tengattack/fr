
#include "stdafx.h"
#include "ScriptSqlite.h"

#include <windows.h>

#include <sqlite3.h>

#include <common/strconv.h>

//#pragma comment(lib, "sqlite3.lib")

CScriptSqlite::CScriptSqlite()
{
}

CScriptSqlite::~CScriptSqlite()
{
	//Close();
}
