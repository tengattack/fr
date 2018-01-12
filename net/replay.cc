
#include "stdafx.h"

#ifdef _SNOW
	#include "../ConsoleControl.h"

	//#include "../zero/VerifyCode.h"
	//extern CVerifyCodeQueue m_queue;

	#define BASE_INFO_PRINT(text, color)	rPrint(text, CONSOLE_TEXT_##color)
	#define INFO_PRINT(text, color)			cPrint(text, CONSOLE_TEXT_##color)
#else
	#ifdef SNOW
		#include <../../Projects/tieba/NewSnow/Snow/dialog/dialog_momo.h>
		#define BASE_INFO_PRINT(text, color)
		#define INFO_PRINT(text, color) if (momo) momo->print(text, MOMO_##color)
	#else
		#define BASE_INFO_PRINT(text, color)
		#define INFO_PRINT(text, color)
	#endif
#endif

#include <tchar.h>

#include <ras.h>
#pragma comment(lib, "rasapi32.lib")

#include <net/net.h>
#include <net/http/TANetBase.h>
#include <common/Buffer.h>

#include <base/string/stringprintf.h>
#include <base/base64.h>

#include "replay_settings.h"
#include "replay.h"

namespace snow {
	CReplay m_replay;
};

CReplay::CReplay()
	: m_replaying(false)
{
}

CReplay::~CReplay()
{
}

int SnowReplayLan(ADD_PARAM())
{
	if (!ReplaySettings::m_setLan.szUrl) return -1;

	int i_ret = 0;
	char szHost[256];
	BOOL bSucceed = FALSE;
	BOOL bNeedAuth = ReplaySettings::m_setLan.szUser && ReplaySettings::m_setLan.szPassword;
	std::string strAuthorization;
	if (bNeedAuth)
	{
		std::string strLogin;
		base::SStringPrintf(&strLogin, "%s:%s", ReplaySettings::m_setLan.szUser, ReplaySettings::m_setLan.szPassword);
		std::string szBase64Encode;
		base::Base64Encode(strLogin, &szBase64Encode);
		if (szBase64Encode.length() > 0)
		{
			base::SStringPrintf(&strAuthorization, "Authorization: Basic %s\r\n", szBase64Encode.c_str());
		}
	}

	lstrcpynA(szHost, ReplaySettings::m_setLan.szUrl, 256);
	DWORD nPort = 80;

	LPCSTR szTmpHost = NULL;
	LPCSTR szTmpHostEnd = NULL;
	char *szPort = NULL;
	//BOOL bSSL = FALSE;
	sockaddr_in sin;
	memset(&sin,0,sizeof(sin));
	sin.sin_family = AF_INET;
	
	if (::_strnicmp(ReplaySettings::m_setLan.szUrl, "http://", 7) == 0)
	{
		nPort = 80;
		szTmpHost = ReplaySettings::m_setLan.szUrl + 7;
	}else if (::_strnicmp(ReplaySettings::m_setLan.szUrl, "https://", 8) == 0){
		nPort = 443;
		szTmpHost = ReplaySettings::m_setLan.szUrl + 8;
	} else {
		nPort = 80;
		szTmpHost = ReplaySettings::m_setLan.szUrl;
	}

	szTmpHostEnd = strstr(szTmpHost, "/");
	if (szTmpHostEnd)
	{
		lstrcpynA(szHost, szTmpHost, szTmpHostEnd - szTmpHost + 1);
	} else {
		if (lstrlenA(szTmpHost) <= 0)
		{
			return 1;	//不合法的URL呢
		}
		lstrcpyA(szHost, szTmpHost);
	}

	szPort = strstr(szHost, ":");
	if (szPort)
	{
		szPort[0] = 0;
		nPort = strtoul((szPort+1), NULL, 10);
	}

	sin.sin_port = htons(nPort);

	hostent* hptr = gethostbyname(szHost);
	if (!hptr) return 2;
	memcpy(&sin.sin_addr.S_un.S_addr,hptr->h_addr,hptr->h_length);  

	SOCKET s = socket(PF_INET,SOCK_STREAM,0);

	if(s == INVALID_SOCKET)   
		return 3;

	SocketDefaultSet(s);

	if(connect(s,(sockaddr*)&sin,sizeof(sin))) 
	{
		closesocket(s);
		return 3;
	}

	if (szPort)	szPort[0] = ':';

	static char szRequestF[] =   
		"%s %s HTTP/1.1\r\n"     
		"Host: %s\r\n"   
		"Accept: */*\r\n"
		"Accept-Language: zh-CN\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"User-Agent: Snow/0.a7 (TengAttack App Engine/0.1)\r\n"   
		"Pragma: no-cache\r\n"   
		"Cache-Control: no-cache\r\n"   
		"Connection: Keep-Alive\r\n"
		"%s\r\n";
		//"Cookie: %s\r\n\r\n";
	std::string szHeader;
	CBuffer buf;
	char *szRetHeader = NULL;
	BOOL bPost = (ReplaySettings::m_setLan.dwFlags & SNOW_REPLAY_LAN_DISCONNECT_POST);
	base::SStringPrintf(&szHeader, szRequestF, bPost ? "POST" : "GET",
		ReplaySettings::m_setLan.szUrlDisconnect, szHost,
		bNeedAuth ? strAuthorization : "");

	if (send(s, szHeader.c_str(), szHeader.length(), 0) == SOCKET_ERROR)
	{
		i_ret = 4;
		goto exit_l;
	}
	if (bPost)
	{
		if (send(s, ReplaySettings::m_setLan.szDataDisconnect, lstrlenA(ReplaySettings::m_setLan.szDataDisconnect), 0) == SOCKET_ERROR)
		{
			i_ret = 5;
			goto exit_l;
		}
	}
	Sleep(500);
	if (!RecvDataEx(s, &buf, &szRetHeader))
	{
		i_ret = 6;
	}
	buf.ClearBuffer();
	if (szRetHeader)
	{
		if (strstr(szRetHeader, "200 OK") != NULL)
		{
			bSucceed = TRUE;
		} else {
			i_ret = 7;
			goto exit_l;
		}
	} else {
		/*i_ret = 7;
		goto exit_l;*/
	}

	
	//连接
	if (ReplaySettings::m_setLan.dwFlags & SNOW_REPLAY_LAN_USELINK)
	{
		if (szRetHeader) free(szRetHeader);
		szRetHeader = NULL;
		
		bPost = (ReplaySettings::m_setLan.dwFlags & SNOW_REPLAY_LAN_CONNECT_POST);
		base::SStringPrintf(&szHeader, szRequestF, bPost ? "POST" : "GET",
			ReplaySettings::m_setLan.szUrlConnect, szHost,
			bNeedAuth ? strAuthorization : "");

		closesocket(s);
		s = socket(PF_INET,SOCK_STREAM,0);
		if(connect(s,(sockaddr*)&sin,sizeof(sin))) 
		{
			closesocket(s);
			return 7;
		}
		if (send(s, szHeader.c_str(), szHeader.length(), 0) == SOCKET_ERROR)
		{
			i_ret = 8;
			goto exit_l;
		}
		if (bPost)
		{
			if (send(s, ReplaySettings::m_setLan.szDataConnect, lstrlenA(ReplaySettings::m_setLan.szDataConnect), 0) == SOCKET_ERROR)
			{
				i_ret = 9;
				goto exit_l;
			}
		}
		Sleep(500);
		if (!RecvDataEx(s, &buf, &szRetHeader))
		{
			i_ret = 10;
		}
		if (szRetHeader)
		{
			if (strstr(szRetHeader, "200 OK") != NULL)
			{
				bSucceed = TRUE;
			} else {
				i_ret = 11;
				goto exit_l;
			}
		} else {
			/*i_ret = 11;
			goto exit_l;*/
			bSucceed = TRUE;
		}
	}
exit_l:
	closesocket(s); 

	if (!bSucceed)
	{
		std::string szInfo = "路由器重拨失败！";
		if (szRetHeader)
		{
			base::StringAppendF(&szInfo, "详细信息：\r\n%s", szRetHeader);
		}
		INFO_PRINT(szInfo.c_str(), COLOR_RED);
		//::MessageBox(NULL, szInfo, _T("错误"), MB_OK | MB_ICONERROR);
	}
	if (szRetHeader) free(szRetHeader);
	return i_ret;
}

//-----断开连接-----
/*BOOL RasClose(HRASCONN  hrasconn)
{
	if(RasHangUp(hrasconn) == 0)	//断开成功
		return TRUE;
	else
		return FALSE;

}*/

int CloseRasConnecting(bool bGetLinkNumber ADD_COMMA_PARAM())
{
	void *pRC;
	DWORD dwSize = 0;
    DWORD dwNumber = 0;
	DWORD singleSize = 0;

	if (m_win_version <= base::win::VERSION_2000)
	{
		//2000以下 500								//XP
		singleSize = sizeof(RASCONN) - sizeof(GUID) - sizeof(LUID) - sizeof(DWORD);
	} else if (m_win_version == base::win::VERSION_XP){
		singleSize = sizeof(RASCONN) - sizeof(GUID);
	} else {
		singleSize = sizeof(RASCONN);
	}

	dwSize = singleSize * 10;
	pRC = malloc(dwSize);
	memset(pRC, 0, dwSize);
	((LPRASCONN)pRC)->dwSize = singleSize;

	//-----枚举活动的连接-----
	DWORD dwRet = RasEnumConnections((LPRASCONN)pRC, &dwSize, &dwNumber);		//枚举已连接的网络
	
	if (bGetLinkNumber)
	{
		free(pRC);
		return dwNumber;
	}

	if(dwRet == ERROR_SUCCESS)
	{
		for(UINT i=0; i<dwNumber; i++)
		{
			RasHangUp(((LPRASCONN)((BYTE *)pRC + i * singleSize))->hrasconn);
			/*if(strcmp(rc[i].szEntryName, "宽带连接") == 0)
			{
				return rc[i].hrasconn;		//返回"我的连接"对应的句柄
			}*/
		}
	} else {
		TCHAR szMessage[256];
		Tstring szInfo = _T("获取连接失败！");
		if (RasGetErrorString((UINT)dwRet, szMessage, 256) == ERROR_SUCCESS)
		{
			szInfo += szMessage;
		}
		INFO_PRINT(szInfo.c_str(), COLOR_RED);
	}

	free(pRC);
	return dwNumber;
}

int CreateADSLEntry(ADD_PARAM())
{
	LPRASENTRY lpRasEntry = NULL;
	DWORD cb = sizeof(RASENTRY);
	DWORD dwBufferSize = 0;
	DWORD dwRet = 0;

	// 取得entry的大小,这句也不知道是不是必须的,因为sizeof(RASENTRY)和这里取到的dwBufferSize是一样的,不过还是Get一下安全点
	RasGetEntryProperties(NULL, _T(""), NULL, &dwBufferSize, NULL, NULL); 
	if (dwBufferSize == 0)
		return -1;

	if (dwBufferSize < cb) cb = dwBufferSize;
	lpRasEntry = (LPRASENTRY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBufferSize);
	if (lpRasEntry == NULL)
		return -2;

	ZeroMemory(lpRasEntry, dwBufferSize);
	lpRasEntry->dwSize = dwBufferSize;
	lpRasEntry->dwfOptions = RASEO_PreviewUserPw|RASEO_RemoteDefaultGateway; // RASEO_PreviewUserPw需要显示ui
	lpRasEntry->dwType = RASET_Broadband;

	lstrcpy(lpRasEntry->szDeviceType, RASDT_PPPoE);
	lstrcpy(lpRasEntry->szDeviceName, _T("azol"));

	lpRasEntry->dwfNetProtocols = RASNP_Ip;
	lpRasEntry->dwFramingProtocol = RASFP_Ppp;

	dwRet = RasSetEntryProperties(NULL, _T("雪凛·宽带连接"), lpRasEntry, dwBufferSize, NULL, 0); // 创建连接
	HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasEntry);

	if (dwRet != ERROR_SUCCESS)
	{
		TCHAR szMessage[256];
		Tstring szInfo;
		base::SStringPrintf(&szInfo, _T("添加宽带连接失败！"), dwRet);
		if (RasGetErrorString((UINT)dwRet, szMessage, 256) == ERROR_SUCCESS)
		{
			szInfo += szMessage;
		}
		INFO_PRINT(szInfo.c_str(), COLOR_RED);
		return -3;
	}

	return 0;
}

void SnowADSLDial(ADD_PARAM())
{
	DWORD singleSize = 0;

	if (m_win_version >= base::win::VERSION_WIN7){
		singleSize = sizeof(RASDIALPARAMS);
	} else {
		singleSize = sizeof(RASDIALPARAMS) - sizeof(DWORD);
	}

	LPRASDIALPARAMS pParams = (LPRASDIALPARAMS)malloc(singleSize);
	HRASCONN handle = NULL;	
	memset(pParams, 0, singleSize);
	pParams->dwSize = singleSize;

	lstrcpy(pParams->szEntryName, _T("雪凛·宽带连接")); //set PPPOE Dial up entry name
	lstrcpy(pParams->szPhoneNumber, _T(""));
	lstrcpy(pParams->szCallbackNumber, _T(""));
	lstrcpy(pParams->szUserName, ReplaySettings::m_setADSL.szUser); //set PPPOE Dial up user name
	lstrcpy(pParams->szPassword, ReplaySettings::m_setADSL.szPassword); //set PPPOE Dial up password
	lstrcpy(pParams->szDomain, _T(""));   

	DWORD dwRet = ::RasSetEntryDialParams(NULL, pParams, FALSE);
	if (dwRet != ERROR_SUCCESS)
	{
		TCHAR szMessage[256];
		Tstring szInfo;
		base::SStringPrintf(&szInfo, _T("设置用户密码信息失败！错误代码：%d\r\n"), dwRet);
		if (RasGetErrorString((UINT)dwRet, szMessage, 256) == ERROR_SUCCESS)
		{
			szInfo += szMessage;
		}
		INFO_PRINT(szInfo.c_str(), COLOR_RED);
	}

	//指定的拨号连接。
	dwRet = RasDial(NULL, NULL, pParams, NULL, NULL, &handle);
	if (dwRet != ERROR_SUCCESS)
	{
		TCHAR szMessage[256];
		Tstring szInfo = _T("重拨失败！");
		if (RasGetErrorString((UINT)dwRet, szMessage, 256) == ERROR_SUCCESS)
		{
			szInfo += szMessage;
		}
		INFO_PRINT(szInfo.c_str(), COLOR_RED);
	}

	free(pParams);
}

void SnowReplayADSL(ADD_PARAM())
{
	if (!ReplaySettings::m_setADSL.szUser || !ReplaySettings::m_setADSL.szPassword)
	{
		MessageBox(NULL, _T("请设置好宽带连接的用户名和密码！"), _T("提示"), MB_OK | MB_ICONINFORMATION);
		return;
	}

	//断开连接
	CloseRasConnecting();
	Sleep(500);	//减少重复IP的可能
	//创建拨号
	CreateADSLEntry();
	//拨号
	SnowADSLDial();

	return;
}

void PrintCurIPAddress(std::string& ip_address)
{
	/*char name[MAX_PATH];
	PHOSTENT hostinfo;
	if(gethostname(name, sizeof(name)) == 0)
	{
		if((hostinfo = gethostbyname(name)) != NULL)
		{
			CString szInfo;
			ip_address = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);
			base::SStringPrintf(&szInfo, "当前IP地址为: %s\r\n", ip_address.c_str());
			rPrint(szInfo, COLOR_GREEN);
		}
	}*/
	CBuffer buf;
	//if (WinSocketDownload("http://fw.qq.com/ipaddress", &buf) == 0)
	//if (WinSocketDownload("http://www.ip.cn/getip2.php?action=getip&ip_url=", &buf) == 0)
	//if (WinSocketDownload("http://city.ip138.com/ip2city.asp", &buf) == 0)
	CTANetBase netbase;
	if (netbase.CurlRequestGet("http://www.ip.cn/getip.php?action=getip&ip_url=&from=web", buf) == TA_NET_OK)
	{
		char *szData = (char *)buf.GetBuffer();
		int iquote = -1;
		if (szData)
		{
			char* start_flag = strstr(szData, "<code>");
			if (start_flag)
			{
				start_flag += 5;
				for (int i = (start_flag - szData);  i < buf.GetBufferLen(); i++)
				{
					//if (szData[i] == '\"')
					if (szData[i] == '>')
					{
						iquote = i + 1;
					} else if (szData[i] == '<' && iquote >= 0) {
						//if (iquote >= 0)
						{
							ip_address.resize(i - iquote + 1);
							lstrcpynA((char *)ip_address.data(), szData + iquote, i - iquote + 1);

							std::string szInfo;
							base::SStringPrintf(&szInfo, "当前IP地址为: %s\r\n", ip_address.c_str());
							BASE_INFO_PRINT(szInfo, COLOR_YELLOW);
							break;
						}/* else {
							iquote = i + 1;
						}*/
					}
				}
			}
		}
	}
}

int SnowReplay(int try_again_times ADD_COMMA_PARAM())
{
	return snow::m_replay.Replay(try_again_times, NULL
#ifdef SNOW
			, momo
#endif
		);
}

typedef struct _SnowReplayInfo {
#ifdef SNOW
	snow::work::WorkState ws;
	void *user;
	snow::work::SNOW_WORK_CALLBACK callback;
#endif
	bool autoclean;
} SnowReplayInfo;

DWORD WINAPI SnowReplyProc(LPVOID lParam)
{
	SnowReplayInfo *wi = (SnowReplayInfo *)lParam;

#ifdef SNOW
	if (wi) {
		//start
		WORK_STATE_SET(wi, START);
		WORK_CALLBACK(wi, START, 0);
	}
#endif
	if (SnowReplay() == 0)
	{
		if (wi) {
#ifdef _SNOW
			if (wi->autoclean)
			{
				//重拨完成后清空
				m_queue.SkipAll();
			}
#endif

#ifdef SNOW
			WORK_CALLBACK(wi, SUCCEED, 0);
#endif
		}
	}
#ifdef SNOW
	else {
		if (wi) {
			WORK_CALLBACK(wi, ERROR, 0);
		}
	}

	if (wi) {
		WORK_STATE_SET(wi, FINISH);
		WORK_CALLBACK(wi, FINISH, 0);
	}
#endif

	if (wi) {
		delete wi;
	}
	return 0;
}

bool SnowReplayAsyn(bool autoclean ADD_COMMA_CALLBACK())
{
	if (snow::m_replay.IsReplaying()) {
#ifdef SNOW
		if (callback) {
			callback(user, SNOW_WS(ERROR), snow::work::kSWETWorking);
			callback(user, SNOW_WS(FINISH), 0);
		}
#endif
		return false;
	}

	SnowReplayInfo *wi = new SnowReplayInfo;
	wi->autoclean = autoclean;
#ifdef SNOW
	wi->user = user;
	wi->callback = callback;
#endif

	::CloseHandle(
		::CreateThread(NULL, 0, SnowReplyProc, wi, NULL, NULL));

	return true;
}

void CReplay::Stop()
{
	m_stopping = true;
}

int CReplay::IPCompare(std::string& old_ip_address, char** new_ip)
{
	int status = 1;

	std::string new_ip_address;
	PrintCurIPAddress(new_ip_address);
	if (new_ip_address == old_ip_address && old_ip_address != "")
	{
		status = 1;
	} else if (new_ip_address == "") {
		status = 2;
	} else {
		//成功
		status = 0;
		if (new_ip)
		{
			*new_ip = _strdup(new_ip_address.c_str());
		}
	}

	return status;
}

int CReplay::ReplayInline(ADD_PARAM())
{
	switch (ReplaySettings::m_bReplayType)
	{
	case SNOW_REPLAY_LAN:
		SnowReplayLan();
		break;
	case SNOW_REPLAY_ADSL:
		SnowReplayADSL();
		break;
	default:
		//INFO_PRINT(_T("错误的请求！"), COLOR_RED);
		return 1;
	}
	return 0;
}

int CReplay::Replay(int try_again_times, char** new_ip ADD_COMMA_PARAM())
{
	//0成功，1失败，2正在
	if (try_again_times <= 0)
	{
		if (m_replaying)
		{
			AutoLock autolock(m_lock);
			return 2;
		}
	}

	AutoLock autolock(m_lock);
	m_replaying = true;
	m_stopping = false;

	INFO_PRINT(_T("雪凛·自动重拨中……"), COLOR_WHITE);

	std::string old_ip_address;
	PrintCurIPAddress(old_ip_address);

	if (ReplayInline() == 1)
	{
		m_replaying = false;
		return 1;
	}

	INFO_PRINT(_T("雪凛·等待重新连接……"), COLOR_WHITE);
	Sleep(1000);

	DWORD lngFlags = 0;
	Tstring szInfo;

	if (m_stopping)
	{
		m_stopping = false;
		m_replaying = false;
		//status = 1; 默认是1
		INFO_PRINT(_T("用户停止重拨，重拨结束。"), COLOR_YELLOW);
		return 1;
	}

	if (InternetGetConnectedState(&lngFlags, 0))
	{
		//connected.
		if (lngFlags & INTERNET_CONNECTION_LAN) {
		//LAN connection.
			ReplaySettings::m_bReplayType = SNOW_REPLAY_LAN;
			//goto finish_l;
		} else if ( lngFlags & INTERNET_CONNECTION_MODEM ) {
		//Modem connection.
			ReplaySettings::m_bReplayType = SNOW_REPLAY_ADSL; 
		} else if ( lngFlags & INTERNET_CONNECTION_PROXY ) {
		//Proxy connection.
			ReplaySettings::m_bReplayType = SNOW_REPLAY_UNKNOW; 
			INFO_PRINT(_T("代理连接。"), COLOR_GREEN);
			m_stopping = false;
			m_replaying = false;
			return 1;
		}
	} else {
		//not connected.
		//ReplaySettings::m_bReplayType = SNOW_REPLAY_UNKNOW; 
	}
	if (ReplaySettings::m_bReplayType == SNOW_REPLAY_ADSL)
	{
		//true 枚举连接的设备数量
		if (CloseRasConnecting(true) <= 0)
		{
			/*INFO_PRINT(_T("关闭连接错误！"), COLOR_RED);
			m_replaying = false;
			return 1;*/
		}
	}

	int status = 1;

	INFO_PRINT(_T("重新获取网络连接信息中（最大重试5次）……"), COLOR_YELLOW);

	for (int i = 0; i <= 5; i++)
	{
		if (m_stopping)
		{
			m_replaying = false;
			m_stopping = false;
			INFO_PRINT(_T("用户停止重拨，重拨结束。"), COLOR_YELLOW);
			
			return 1;
			break;
		}

		switch (IPCompare(old_ip_address, new_ip))
		{
		case 0:
			status = 0;
			goto exit_l;
			break;
		case 1:
			if (i >= 5)
			{
				INFO_PRINT(_T("重拨后IP地址未改变！"), COLOR_RED);
				break;
			}
			base::SStringPrintf(&szInfo, _T("重拨后IP地址未改变！（重试，第%d次）"), i + 1);
			if (ReplayInline() == 1)
			{
				goto exit_l;
			}
			break;
		case 2:
			if (i >= 5) break;
			base::SStringPrintf(&szInfo, _T("重拨后无法获得新IP地址！（重试，第%d次）"), i + 1);
			break;
		}

		INFO_PRINT(szInfo.c_str(), COLOR_RED);

		Sleep(2000);
	}

exit_l:
	if (status == 0) {
		INFO_PRINT(_T("获取完成！"), COLOR_GREEN);
	} else {
		INFO_PRINT(_T("无法获取重拨后新IP地址！"), COLOR_RED);
	}

	m_replaying = false;
	return status;
}