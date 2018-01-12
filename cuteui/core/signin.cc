
#include "signin.h"
#include <incMobileEngine/stringfmt.h>
#include "base/string/stringprintf.h"

namespace core{
	CSignIn::CSignIn()
		: m_sqlite(NULL)
		, m_bInit(false)
	{
	}
	CSignIn::~CSignIn()
	{
		Uninit();
	}

	void CSignIn::Uninit()
	{
		if (m_sqlite)
		{
			sqlite3_close(m_sqlite);
			m_sqlite = NULL;
		}
	}

	bool CSignIn::Init()
	{
		//Path
		char m_szDBPath[MAX_PATH];
		m_szDBPath[0] = 0;
		::GetModuleFileNameA(NULL, m_szDBPath, MAX_PATH);
		for (int i = lstrlenA(m_szDBPath); i > 0; i--)
		{
			if (m_szDBPath[i] == '\\' || m_szDBPath[i] == '/')
			{
				m_szDBPath[i+1] = 0;
				break;
			}
		}
		lstrcatA(m_szDBPath, "userdata.db");

		//需要utf8字符
		char *utf8path = MbToUtf8String(m_szDBPath);
		m_dbpath = utf8path;
		free(utf8path);

		//Init SQLite
		int ret = sqlite3_open_v2(m_dbpath.c_str(), &m_sqlite, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
			//sqlite3_open(m_szUsersPath, &m_sqlite);
		if (ret != SQLITE_OK)
		{
			std::string szInfo;
			base::SStringPrintf(&szInfo, "打开用户数据库文件失败！\r\n错误描述：\r\n%s", sqlite3_errmsg(m_sqlite));
			MessageBoxA(NULL, szInfo.c_str(), "错误", MB_OK | MB_ICONERROR);
			return false;
		}

		char *errMsg = NULL;
		ret = sqlite3_exec(m_sqlite, 
			"CREATE TABLE Users([ID] INTEGER PRIMARY KEY AUTOINCREMENT, [Username] VARCHAR(50), [Password] VARCHAR(50), [Tiebas] VARCHAR(2048), [Posts] VARCHAR(2048), [LastDate] VARCHAR(10), [Flags] INTEGER default 0);", 
			0, 0, &errMsg);
		if (ret != SQLITE_OK)
		{
			if (strstr(errMsg, "already exist") == NULL)
			{
				std::string szInfo;
				base::SStringPrintf(&szInfo, "创建用户数据库表失败！\r\n错误描述：\r\n%s", errMsg);
				MessageBoxA(NULL, szInfo.c_str(), "错误", MB_OK | MB_ICONERROR);
			}
		}
		if (errMsg) sqlite3_free(errMsg);

		GetUsers();
		m_bInit = true;
		return true;
	}

	int CSignIn::ScanUsers(void* data, int n_columns, char** column_values, char** column_names)
	{
		if (n_columns == 7)
		{
			//[Tiebas],[Posts],[LastDate]
			CSignIn *pMgr = (CSignIn *)data;
			TA_SIGNIN_USERS tu;
			tu.iIndex = atoi(column_values[0]);
			tu.szUsername = column_values[1];
			tu.szPassword = column_values[2];
			tu.szTiebas = column_values[3];
			tu.szPosts = column_values[4];
			tu.szLastDate = column_values[5];
			tu.flags = atoi(column_values[6]);
			pMgr->vecUsers.push_back(tu);
		}
		return 0;
	}

	int CSignIn::GetUsers()
	{
		char *errMsg = NULL;
		int ret = sqlite3_exec(m_sqlite, "SELECT * FROM [Users];", ScanUsers, this, &errMsg);

		if (ret != SQLITE_OK)
		{
			std::string szInfo;
			base::SStringPrintf(&szInfo, "获取用户数据失败！\r\n错误描述：\r\n%s", errMsg);
			MessageBoxA(NULL, szInfo.c_str(), "错误", MB_OK | MB_ICONERROR);
		}
		if (errMsg) sqlite3_free(errMsg);
		return vecUsers.size();
	}

	int CSignIn::InsertID(LPCSTR lpszUsername, LPCSTR lpszPassword, LPCSTR lpszTiebas, LPCSTR lpszPosts, LPCSTR lpszLastDate, DWORD flags)
	{
		for (unsigned long i = 0; i < vecUsers.size(); i++)
		{
			if (lstrcmpiA(vecUsers[i].szUsername.c_str(), lpszUsername) == 0)
			{
				vecUsers[i].szLastDate = lpszLastDate;
				vecUsers[i].szTiebas = lpszTiebas;
				vecUsers[i].szPosts = lpszPosts;
				vecUsers[i].szPassword = lpszPassword;
				vecUsers[i].flags = flags;
				if (UpdateIDInfo(i))
				{
					return i;
				} else {
					break;
					//再添加一个吧
				}
			}
		}

		std::string strConn;
		base::SStringPrintf(&strConn, "INSERT INTO [Users]([Username],[Password],[Tiebas],[Posts],[LastDate],[Flags]) VALUES ('%s','%s','%s','%s','%s',%d);", 
				lpszUsername, lpszPassword, lpszTiebas, lpszPosts, lpszLastDate, flags);
		char *errMsg = NULL;
		int ret = sqlite3_exec(m_sqlite, 
			strConn.c_str(), 
			0, 0, &errMsg);

		if (ret != SQLITE_OK)
		{
			std::string szInfo;
			base::SStringPrintf(&szInfo, "添加用户数据失败！\r\n错误描述：\r\n%s", errMsg);
			MessageBoxA(NULL, szInfo.c_str(), "错误", MB_OK | MB_ICONERROR);
		} else {
			//add
			TA_SIGNIN_USERS tu;
			tu.iIndex = 0;
			if (vecUsers.size() > 0)
			{
				tu.iIndex = vecUsers[vecUsers.size()-1].iIndex + 1;
			}
			tu.szUsername = lpszUsername;
			tu.szPassword = lpszPassword;
			tu.szTiebas = lpszTiebas;
			tu.szPosts = lpszPosts;
			tu.szLastDate = lpszLastDate;
			
			tu.flags = flags;
			vecUsers.push_back(tu);
		}
		if (errMsg) sqlite3_free(errMsg);
		if (ret == SQLITE_OK)
		{
			return vecUsers.size() - 1;
		} else {
			return -1;
		}
	}

	bool CSignIn::DeleteID(int iIndex)
	{
		std::string strConn;
		base::SStringPrintf(&strConn, "DELETE FROM [Users] WHERE ID=%d", vecUsers[iIndex].iIndex);
		char *errMsg = NULL;
		int ret = sqlite3_exec(m_sqlite, 
			strConn.c_str(), 
			0, 0, &errMsg);

		if (ret != SQLITE_OK)
		{
			std::string szInfo;
			base::SStringPrintf(&szInfo, "删除用户数据失败！\r\n错误描述：\r\n%s", errMsg);
			MessageBoxA(NULL, szInfo.c_str(), "错误", MB_OK | MB_ICONERROR);
		} else {
			vecUsers.erase(vecUsers.begin() + iIndex);
		}
		if (errMsg) sqlite3_free(errMsg);
		return (ret == SQLITE_OK);
	}

	bool CSignIn::UpdateIDInfo(int iIndex)
	{
		//[Tiebas],[Posts],[LastDate]
		std::string strConn;
		base::SStringPrintf(&strConn, "UPDATE [Users] SET [Password] = '%s' WHERE ID=%d; "
					   "UPDATE [Users] SET [Tiebas] = '%s' WHERE ID=%d; "
					   "UPDATE [Users] SET [Posts] = '%s' WHERE ID=%d; "
					   "UPDATE [Users] SET [LastDate] = '%s' WHERE ID=%d; "
					   "UPDATE [Users] SET [Flags] = %d WHERE ID=%d; ",
			vecUsers[iIndex].szPassword.c_str(),
			vecUsers[iIndex].iIndex,
			vecUsers[iIndex].szTiebas.c_str(),
			vecUsers[iIndex].iIndex,
			vecUsers[iIndex].szPosts.c_str(),
			vecUsers[iIndex].iIndex,
			vecUsers[iIndex].szLastDate.c_str(),
			vecUsers[iIndex].iIndex,
			vecUsers[iIndex].flags, 
			vecUsers[iIndex].iIndex);

		char *errMsg = NULL;
		int ret = sqlite3_exec(m_sqlite, 
			strConn.c_str(), 
			0, 0, &errMsg);

		if (ret != SQLITE_OK)
		{
			std::string szInfo;
			base::SStringPrintf(&szInfo, "更新用户数据失败！\r\n错误描述：\r\n%s", errMsg);
			MessageBoxA(NULL, szInfo.c_str(), "错误", MB_OK | MB_ICONERROR);
		}
		if (errMsg) sqlite3_free(errMsg);
		return (ret == SQLITE_OK);
	}

	bool CSignIn::UpdateIDDateInfo(int iIndex)
	{
		std::string strConn;
		base::SStringPrintf(&strConn, 
					   "UPDATE [Users] SET [LastDate] = '%s' WHERE ID=%d; ",
			vecUsers[iIndex].szLastDate.c_str(),
			vecUsers[iIndex].iIndex);

		char *errMsg = NULL;
		int ret = sqlite3_exec(m_sqlite, 
			strConn.c_str(), 
			0, 0, &errMsg);

		if (ret != SQLITE_OK)
		{
			std::string szInfo;
			base::SStringPrintf(&szInfo, "更新用户数据失败！\r\n错误描述：\r\n%s", errMsg);
			MessageBoxA(NULL, szInfo.c_str(), "错误", MB_OK | MB_ICONERROR);
		}
		if (errMsg) sqlite3_free(errMsg);
		return (ret == SQLITE_OK);
	}
};