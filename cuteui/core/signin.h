
#ifndef _SNOW_CUTE_CORE_SIGNIN_H_
#define _SNOW_CUTE_CORE_SIGNIN_H_ 1

#include "../base/common.h"
#include <sqlite3.h>
#include <vector>

namespace core{
	
	typedef struct _TA_SIGNIN_USERS {
		int iIndex;
		std::string szUsername;
		std::string szPassword;
		std::string szTiebas;
		std::string szPosts;
		std::string szLastDate;
		int flags;
	} TA_SIGNIN_USERS;

	class CSignIn{
	public:
		CSignIn();
		~CSignIn();

		bool Init();
		void Uninit();

		inline int GetUserCount()
		{
			return (int)vecUsers.size();
		}
		inline TA_SIGNIN_USERS& GetUser(int iIndex)
		{
			return vecUsers[iIndex];
		}

		int GetUsers();

		int InsertID(LPCSTR lpszUsername, LPCSTR lpszPassword, LPCSTR lpszTiebas, LPCSTR lpszPosts, LPCSTR lpszLastDate, DWORD flags);
		bool UpdateIDInfo(int iIndex);
		bool UpdateIDDateInfo(int iIndex);
		bool DeleteID(int iIndex);

		static int ScanUsers(void* data, int n_columns, char** column_values, char** column_names);

	protected:
		//SQLite
		sqlite3 *m_sqlite;
		bool m_bInit;
		std::string m_dbpath;
		std::vector<TA_SIGNIN_USERS> vecUsers;
	};
};
#endif