#pragma once

#include <vector>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "TANetBase.h"

typedef struct _TA_COOKIE_SESSION{
	char *szNote;
	char *szData;
}TA_COOKIE_SESSION;

class CTACookiesManger
{
public:
	friend class CTANetBase;

	CTACookiesManger(void)
		: m_ptr_net_base_(NULL)
	{
	};

	virtual ~CTACookiesManger(void);

	void bind(CTANetBase *pNetBase, bool bAutoBuild = true)
	{
		m_ptr_net_base_ = pNetBase;
		m_bAutoBuild = bAutoBuild;
	};

	void add(const char *node, const char *data);	//it will not autobuild
	bool explain(const char *szRetHeader, bool bMap = false);
	bool refresh();
	bool build();
	void clear();

	inline int count() {
		return (int)m_vecSession.size();
	}

	static int GetCookiesFormRetHeader(LPCSTR lpszRetHeader, LPSTR lpszCookies);

	LPCSTR GetSession(LPCSTR note);
	
protected:
	std::vector<TA_COOKIE_SESSION> m_vecSession;

	CTANetBase *m_ptr_net_base_;
	bool m_bAutoBuild;

	int FindNote(LPCSTR szNote);
	void AddNote(LPCSTR szNote, LPCSTR szData);
	void ModifyNote(int iIndex, LPCSTR szData);

	bool CheckLine(char *szLine);
};

