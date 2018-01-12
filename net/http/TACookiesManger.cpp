
#include "TACookiesManger.h"

CTACookiesManger::~CTACookiesManger(void)
{
	clear();
}

int CTACookiesManger::FindNote(LPCSTR szNote)
{
	int iSessionCount = count();
	for (int i = 0; i < iSessionCount; i++)
	{
		if (lstrcmpA(szNote, m_vecSession[i].szNote) == 0)
		{
			return i;
		}
	}
	return -1;
}

void CTACookiesManger::clear()
{
	int iSessionCount = count();
	for (int i = 0; i < iSessionCount; i++)
	{
		free(m_vecSession[i].szNote);
		free(m_vecSession[i].szData);
	}
	m_vecSession.clear();
}

bool CTACookiesManger::CheckLine(char *szLine)
{
	char *szTmp, *szData, *szDataEnd;

	if (szTmp = strstr(szLine, "max-age="))
	{
		szTmp[0] = 0;
	}
	if (szTmp = strstr(szLine, "expires="))
	{
		szTmp[0] = 0;
	}
	if (szTmp = strstr(szLine, "path="))
	{
		szTmp[0] = 0;
	}

	szData = strstr(szLine, "=");
	if (!szData) return false;

	szData[0] = 0;
	szData++;

	szDataEnd = strstr(szData, ";");
	if (!szDataEnd) return false;
	szDataEnd[0] = 0;

	if (lstrlenA(szData) <= 0) return 0;

	int iIndex = FindNote(szLine);
	if (iIndex >= 0)
	{
		ModifyNote(iIndex, szData);
	} else {
		AddNote(szLine, szData);
	}

	return true;
}

void CTACookiesManger::AddNote(LPCSTR szNote, LPCSTR szData)
{
	TA_COOKIE_SESSION cs;
	cs.szNote = _strdup(szNote);
	cs.szData = _strdup(szData);
	m_vecSession.push_back(cs);
}

void CTACookiesManger::ModifyNote(int iIndex, LPCSTR szData)
{
	free(m_vecSession[iIndex].szData);
	m_vecSession[iIndex].szData = _strdup(szData);
}

bool CTACookiesManger::refresh()
{
	if (!m_ptr_net_base_) {
		return false;
	}

	clear();
	const char *cookie_ = m_ptr_net_base_->GetCookie();
	if (cookie_) {
		char *map_cookie = _strdup(cookie_);

		char *szSession = (char *)map_cookie;
		char *szSessionEnd = NULL;
		char *szSessionData = NULL;

		while (true)
		{
			szSessionEnd = strstr(szSession, ";");
			
			if (szSessionEnd) szSessionEnd[0] = 0;

			szSessionData = strstr(szSession, "=");
			if (szSessionData) {
				szSessionData[0] = 0;
				szSessionData++;
			}

			if (lstrlenA(szSession) > 0) {
				AddNote(szSession, szSessionData ? szSessionData : "");
			}
			if (!szSessionEnd) {
				break;
			}

			szSession = szSessionEnd + 1;
			if (szSession[0] == ' ') szSession++;
		}
		free(map_cookie);
	}

	return true;
}

void CTACookiesManger::add(const char *node, const char *data)
{
	int iIndex = FindNote(node);
	if (iIndex >= 0)
	{
		ModifyNote(iIndex, data);
	} else {
		AddNote(node, data);
	}
}

bool CTACookiesManger::explain(const char *szRetHeader, bool bMap)
{
	bool b_ret = false;
	char *szMap = (bMap ? _strdup(szRetHeader) : (char *)szRetHeader);

	char *szLineEnd = (char *)szMap;
	char *szSetCookies;

	while ((szSetCookies = strstr(szLineEnd, "Set-Cookie:")) != NULL)
	{
		szSetCookies += 11;
		if (szSetCookies[0] == ' ') szSetCookies++;

		szLineEnd = strstr(szSetCookies, "\r\n");
		if (szLineEnd)
		{
			szLineEnd[0] = 0;
			szLineEnd++;
		}

		if (CheckLine(szSetCookies))
		{
			b_ret = true;
		}
		if (!szLineEnd)
		{
			break;
		}
	}

	if (bMap) free(szMap);
	if (m_bAutoBuild && b_ret) build();

	return b_ret;
}

bool CTACookiesManger::build()
{
	int iSessionCount = count();
	bool b_ret = false;

	std::string& str_cookie_ = m_ptr_net_base_->GetStringCookie();

	str_cookie_.clear();

	for (int i = 0; i < iSessionCount; i++)
	{
		b_ret = true;

		str_cookie_ += m_vecSession[i].szNote;
		str_cookie_ += "=";
		str_cookie_ += m_vecSession[i].szData;
		str_cookie_ += "; ";
	}

	//m_ptr_net_base_->SetCookie(str_cookie_.c_str());

	return b_ret;
}

LPCSTR CTACookiesManger::GetSession(LPCSTR note)
{
	int iIndex = FindNote(note);
	if (iIndex >= 0)
	{
		return m_vecSession[iIndex].szData;
	} else {
		return NULL;
	}
}

int CTACookiesManger::GetCookiesFormRetHeader(LPCSTR lpszRetHeader, LPSTR lpszCookies)
{
	bool b_ret = false;
	lpszCookies[0] = 0;
	char *szMap = _strdup(lpszRetHeader);
	char *szLineEnd = (char *)szMap;
	char *szSetCookies, *szTmp;

	while ((szSetCookies = strstr(szLineEnd, "Set-Cookie:")) != NULL)
	{
		szSetCookies += 11;
		if (szSetCookies[0] == ' ') szSetCookies++;

		szLineEnd = strstr(szSetCookies, "\r\n");
		if (szLineEnd)
		{
			b_ret = true;

			szLineEnd[0] = 0;

			if (szTmp = strstr(szSetCookies, "max-age="))
			{
				szTmp[0] = 0;
			}
			if (szTmp = strstr(szSetCookies, "expires="))
			{
				szTmp[0] = 0;
			}
			if (szTmp = strstr(szSetCookies, "path="))
			{
				szTmp[0] = 0;
			}
			
			lstrcatA(lpszCookies, szSetCookies);

			szLineEnd++;

		} else {

			b_ret = true;

			lstrcatA(lpszCookies, szSetCookies);
			break;
		}
	}

	free(szMap);

	return (b_ret ? 0 : -1);
}