
#include "stdafx.h"

#include <string>
#include <base/string/stringprintf.h>
#include <base/file/file.h>
#include <base/file/filedata.h>

#include <filecommon/file_path.h>

#include <shlobj.h>
#pragma comment(lib, "shell32.lib")

#include "replay_settings.h"

base::win::Version m_win_version = base::win::VERSION_XP;

namespace ReplaySettings {
	Tstring m_datapath;
	BYTE m_bReplayType;
	TA_SNOW_LAN_REPLAY_SETTINGS m_setLan;
	bool m_lan_changed = true;
	TA_SNOW_ADSL_REPLAY_SETTINGS m_setADSL;
	bool m_adsl_changed = true;
};

SnowNetState SnowGetNetState()
{
	SnowNetState state = kSNSNotConnected;

	DWORD lngFlags = 0;
	if (InternetGetConnectedState(&lngFlags, 0))
	{
		//connected.
		if (lngFlags & INTERNET_CONNECTION_LAN) {
		//LAN connection.
			ReplaySettings::m_bReplayType = SNOW_REPLAY_LAN;
			state = kSNSLan;

		} else if ( lngFlags & INTERNET_CONNECTION_MODEM ) {
		//Modem connection.
			ReplaySettings::m_bReplayType = SNOW_REPLAY_ADSL;
			state = kSNSModem;

		} else if ( lngFlags & INTERNET_CONNECTION_PROXY ) {
		//Proxy connection.
			ReplaySettings::m_bReplayType = SNOW_REPLAY_UNKNOW; 
			state = kSNSProxy;
		}
	} else {
		//not connected.
		ReplaySettings::m_bReplayType = SNOW_REPLAY_UNKNOW;
		state = kSNSNotConnected;
	}

	return state;
}

void SetLanSettingsChanged(bool changed)
{
	ReplaySettings::m_lan_changed = changed;
}

void SetADSLSettingsChanged(bool changed)
{
	ReplaySettings::m_adsl_changed = changed;
}

void ClearReplaySettings()
{
	memset(&ReplaySettings::m_setLan, 0, sizeof(TA_SNOW_LAN_REPLAY_SETTINGS));
	memset(&ReplaySettings::m_setADSL, 0, sizeof(TA_SNOW_ADSL_REPLAY_SETTINGS));
}

void InitReplaySettings(LPCTSTR datapath)
{
	if (datapath) {
		ReplaySettings::m_datapath = datapath;
		if (!HaveRightSlash(ReplaySettings::m_datapath.c_str())) {
			 ReplaySettings::m_datapath += _T("/");
		}
	}

	m_win_version = base::win::GetVersion();

	ClearReplaySettings();
	
	ReplaySettings::m_bReplayType = SNOW_REPLAY_UNKNOW;

	ReplaySettings::m_setLan.dwFlags = SNOW_REPLAY_LAN_DISCONNECT_GET | SNOW_REPLAY_LAN_CONNECT_GET;

	SnowGetNetState();
}

void FreeReplaySettings()
{
	switch (ReplaySettings::m_bReplayType)
	{
	case SNOW_REPLAY_LAN:
		if (ReplaySettings::m_setLan.szUser)
			free(ReplaySettings::m_setLan.szUser);
		if (ReplaySettings::m_setLan.szPassword)
			free(ReplaySettings::m_setLan.szPassword);
		if (ReplaySettings::m_setLan.szUrl)
			free(ReplaySettings::m_setLan.szUrl);
		if (ReplaySettings::m_setLan.szUrlDisconnect)
			free(ReplaySettings::m_setLan.szUrlDisconnect);
		if (ReplaySettings::m_setLan.szUrlConnect)
			free(ReplaySettings::m_setLan.szUrlConnect);
		if (ReplaySettings::m_setLan.szDataDisconnect)
			free(ReplaySettings::m_setLan.szDataDisconnect);
		if (ReplaySettings::m_setLan.szDataConnect)
			free(ReplaySettings::m_setLan.szDataConnect);

		ReplaySettings::m_setLan.dwFlags = SNOW_REPLAY_LAN_DISCONNECT_GET | SNOW_REPLAY_LAN_CONNECT_GET;

		break;
	case SNOW_REPLAY_ADSL:
		if (ReplaySettings::m_setADSL.szUser)
			free(ReplaySettings::m_setADSL.szUser);
		if (ReplaySettings::m_setADSL.szPassword)
			free(ReplaySettings::m_setADSL.szPassword);
		break;
	}

	ClearReplaySettings();
}

void SnowLanSettings(BOOL bLoad)
{
	TCHAR szLanSettings[MAX_PATH];
	lstrcpy(szLanSettings, ReplaySettings::m_datapath.c_str());
	lstrcat(szLanSettings, _T("ReplayLanSettings.txt"));
	DWORD dwData = 0;
	if (bLoad)
	{
		base::CFile file;
		if (file.Open(base::kFileRead, szLanSettings))
		{
			base::CFileData fd;
			if (fd.Read(file))
			{
				std::string szData;
				fd.ToText(szData);

				char *dataStart = (char *)szData.c_str();
				char *dataEnd = strstr(dataStart, "\r\n");
				char *dataEnd2;
				if (dataEnd == NULL)
				{
					goto exit_l;
				}

				//用户名
				dataEnd[0] = 0;
				if (dataStart != dataEnd)
					ReplaySettings::m_setLan.szUser = _strdup(dataStart);
				
				//密码
				dataStart = dataEnd + 2;
				dataEnd = strstr(dataStart, "\r\n");
				if (dataEnd == NULL)
				{
					goto exit_l;
				}
				dataEnd[0] = 0;
				if (dataStart != dataEnd)
					ReplaySettings::m_setLan.szPassword = _strdup(dataStart);

				//路由器网址
				dataStart = dataEnd + 2;
				dataEnd = strstr(dataStart, "\r\n");
				if (dataEnd == NULL)
				{
					goto exit_l;
				}
				dataEnd[0] = 0;
				if (dataStart != dataEnd)
					ReplaySettings::m_setLan.szUrl = _strdup(dataStart);

				//断线地址
				dataStart = dataEnd + 2;
				dataEnd = strstr(dataStart, "\r\n");
				if (dataEnd == NULL)
				{
					goto exit_l;
				}
				dataEnd[0] = 0;
				dataEnd2 = strstr(dataStart, " ");
				if (dataEnd2 == NULL)
				{
					goto exit_l;
				}
				dataEnd2[0] = 0;
				if (lstrcmpA(dataStart, "POST") == 0) ReplaySettings::m_setLan.dwFlags |= SNOW_REPLAY_LAN_DISCONNECT_POST;
				dataStart = dataEnd2 + 1;
				if (dataStart != dataEnd)
					ReplaySettings::m_setLan.szUrlDisconnect = _strdup(dataStart);

				dataStart = dataEnd + 2;
				if (ReplaySettings::m_setLan.dwFlags & SNOW_REPLAY_LAN_DISCONNECT_POST)
				{
					dataEnd = strstr(dataStart, "\r\n\r\nEND\r\n");
					if (dataEnd == NULL)
					{
						goto exit_l;
					}
					dataEnd[0] = 0;
					if (dataStart != dataEnd)
						ReplaySettings::m_setLan.szDataDisconnect = _strdup(dataStart);
					dataEnd += 7;	//移到最后的换行
				}

				//连线地址
				dataStart = dataEnd + 2;
				dataEnd = strstr(dataStart, "\r\n");
				if (dataEnd == NULL)
				{
					goto exit_l;
				}
				dataEnd[0] = 0;
				if (lstrcmpA(dataStart, "USELINK") == 0) ReplaySettings::m_setLan.dwFlags |= SNOW_REPLAY_LAN_USELINK;

				dataStart = dataEnd + 2;
				dataEnd = strstr(dataStart, "\r\n");
				if (dataEnd == NULL)
				{
					goto exit_l;
				}
				dataEnd[0] = 0;

				dataEnd2 = strstr(dataStart, " ");
				if (dataEnd2 == NULL)
				{
					goto exit_l;
				}
				dataEnd2[0] = 0;
				if (lstrcmpA(dataStart, "POST") == 0) ReplaySettings::m_setLan.dwFlags |= SNOW_REPLAY_LAN_CONNECT_POST;
				dataStart = dataEnd2 + 1;
				if (dataStart != dataEnd)
					ReplaySettings::m_setLan.szUrlConnect = _strdup(dataStart);

				dataStart = dataEnd + 2;
				if (ReplaySettings::m_setLan.dwFlags & SNOW_REPLAY_LAN_CONNECT_POST)
				{
					dataEnd = strstr(dataStart, "\r\n\r\nEND\r\n");
					if (dataEnd == NULL)
					{
						goto exit_l;
					}
					dataEnd[0] = 0;
					if (dataStart != dataEnd)
						ReplaySettings::m_setLan.szDataConnect = _strdup(dataStart);
					dataEnd += 7;
				}

exit_l:
				;
			}
			file.Close();
		}	//if (file.Open
	}
	else
	{
		if (!ReplaySettings::m_lan_changed) {
			return;
		}
		base::CFile file;
		if (file.Open(base::kFileCreate, szLanSettings))
		{
			std::string szData;
			base::StringAppendF(&szData, "%s\r\n", ReplaySettings::m_setLan.szUser ? ReplaySettings::m_setLan.szUser : "");
			base::StringAppendF(&szData, "%s\r\n", ReplaySettings::m_setLan.szPassword ? ReplaySettings::m_setLan.szPassword : "");
			
			base::StringAppendF(&szData, "%s\r\n", ReplaySettings::m_setLan.szUrl ? ReplaySettings::m_setLan.szUrl : "");
			BOOL bPost = (ReplaySettings::m_setLan.dwFlags & SNOW_REPLAY_LAN_DISCONNECT_POST);
			base::StringAppendF(&szData, "%s %s\r\n", 
				bPost ? "POST" : "GET",
				ReplaySettings::m_setLan.szUrlDisconnect ? ReplaySettings::m_setLan.szUrlDisconnect : "");
			if (bPost)
			{
				base::StringAppendF(&szData, "%s\r\n\r\nEND\r\n", 
					ReplaySettings::m_setLan.szDataDisconnect ? ReplaySettings::m_setLan.szDataDisconnect : "");
			}
			base::StringAppendF(&szData, "%s\r\n", (ReplaySettings::m_setLan.dwFlags & SNOW_REPLAY_LAN_USELINK) ? "USELINK" : "");
			bPost = ReplaySettings::m_setLan.dwFlags & SNOW_REPLAY_LAN_CONNECT_POST;
			base::StringAppendF(&szData, "%s %s\r\n", 
				bPost ? "POST" : "GET",
				ReplaySettings::m_setLan.szUrlConnect ? ReplaySettings::m_setLan.szUrlConnect : "");

			if (bPost)
			{
				base::StringAppendF(&szData, "%s\r\n\r\nEND\r\n", 
					ReplaySettings::m_setLan.szDataConnect ? ReplaySettings::m_setLan.szDataConnect : "");
			}

			file.Write((unsigned char *)szData.c_str(), szData.length());
			file.Close();
		}
	}
}

void SnowADSLSettings(BOOL bLoad)
{
	TCHAR szADSLSettings[MAX_PATH];
	szADSLSettings[0] = 0;
	LPITEMIDLIST pidl;
	//Const APPDATA = &H1A& 'Application Data
	SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidl);
	SHGetPathFromIDList(pidl, szADSLSettings);
	::SHFree(pidl);

	int nLen = lstrlen(szADSLSettings);
	if (szADSLSettings[nLen - 1] != '\\')
	{
		szADSLSettings[nLen] = '\\';
		szADSLSettings[nLen+1] = 0;
	}

	lstrcat(szADSLSettings, _T("TADataFiles"));
	CreateDir(szADSLSettings);

	lstrcat(szADSLSettings, _T("\\ReplayADSLSettings.txt"));

	DWORD dwData = 0;
	if (bLoad)
	{
		base::CFile file;
		if (file.Open(base::kFileRead, szADSLSettings))
		{
			base::CFileData fd;
			if (fd.Read(file))
			{
				Tstring szData;
				fd.ToText(szData);

				TCHAR *dataStart = (TCHAR *)szData.c_str();
				TCHAR *dataEnd = _tcsstr(dataStart, _T("\r\n"));

				if (dataEnd == NULL)
				{
					goto exit_l;
				}

				//用户名
				dataEnd[0] = 0;
				if (dataStart != dataEnd)
					ReplaySettings::m_setADSL.szUser = _tcsdup(dataStart);
				
				//密码
				dataStart = dataEnd + 2;
				dataEnd = _tcsstr(dataStart, _T("\r\n"));
				if (dataEnd == NULL)
				{
					goto exit_l;
				}
				dataEnd[0] = 0;
				if (dataStart != dataEnd)
					ReplaySettings::m_setADSL.szPassword = _tcsdup(dataStart);

exit_l:
				;
			}
			file.Close();
		}	//if (file.Open
	} else {
		if (!ReplaySettings::m_adsl_changed) {
			return;
		}
		base::CFile file;
		if (file.Open(base::kFileCreate, szADSLSettings))
		{
			Tstring szData;
			base::SStringPrintf(&szData, _T("%s\r\n"), ReplaySettings::m_setADSL.szUser ? ReplaySettings::m_setADSL.szUser : _T(""));
			base::StringAppendF(&szData, _T("%s\r\n"), ReplaySettings::m_setADSL.szPassword ? ReplaySettings::m_setADSL.szPassword : _T(""));

#ifdef _UNICODE
			file.Write((unsigned char *)"\xff\xfe", 2);
#endif
			file.Write((unsigned char *)szData.c_str(), szData.length() * sizeof(TCHAR));

			file.Close();
		}
	}
}

void LoadReplaySettings()
{
	FreeReplaySettings();

	switch (ReplaySettings::m_bReplayType)
	{
	case SNOW_REPLAY_LAN:
		SnowLanSettings(TRUE);
		break;
	case SNOW_REPLAY_ADSL:
		SnowADSLSettings(TRUE);
		break;
	}
}

void SaveReplaySettings()
{
	switch (ReplaySettings::m_bReplayType)
	{
	case SNOW_REPLAY_LAN:
		SnowLanSettings(FALSE);
		break;
	case SNOW_REPLAY_ADSL:
		SnowADSLSettings(FALSE);
		break;
	}
}