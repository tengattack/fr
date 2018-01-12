
#include "stdafx.h"

#include <common/Buffer.h>
#include <net/http/TANetBase.h>

extern bool RecvDataEx(SOCKET sock, CBuffer *buf, char **szHeader = NULL);
extern bool RecvDataEx(SOCKET sock, char **lpszData);

int WinSocketDownloadEx(LPCSTR lpszUrl, LPCSTR lpszUserAgent, LPCSTR lpszCookie, CBuffer *pBuf, char **szheader)
{
	int i_ret = 0;
	char szHost[256];
	LPCSTR szTmpHost = NULL;
	LPCSTR szTmpHostEnd = NULL;

	//BOOL bSSL = FALSE;
	sockaddr_in sin;
	memset(&sin,0,sizeof(sin));
	sin.sin_family = AF_INET;
	
	if (::_strnicmp(lpszUrl, "http://", 7) == 0)
	{
		sin.sin_port = htons(80);
		szTmpHost = lpszUrl + 7;
	}else if (::_strnicmp(lpszUrl, "https://", 8) == 0){
		sin.sin_port = htons(443);
		szTmpHost = lpszUrl + 8;
	} else {
		return 1;
	}

	szTmpHostEnd = strstr(szTmpHost, "/");
	if (szTmpHostEnd)
	{
		lstrcpynA(szHost, szTmpHost, szTmpHostEnd - szTmpHost + 1);
	} else {
		if (lstrlenA(szTmpHost) <= 0)
		{
			return 2;	//不合法的URL呢
		}
		lstrcpyA(szHost, szTmpHost);
	}

	SOCKET s = socket(PF_INET,SOCK_STREAM,0);
	if(s == INVALID_SOCKET)   
		return 3;

	int nTime = 30000; 
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTime, sizeof(nTime)); 
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(nTime));

	hostent* hptr = gethostbyname(szHost);
	memcpy(&sin.sin_addr.S_un.S_addr,hptr->h_addr,hptr->h_length);  
	if(connect(s,(sockaddr*)&sin,sizeof(sin))) 
	{
		closesocket(s);
		return 3;
	}

	//char* h = NULL;
	static char szRequestF[] =   
		"GET %s HTTP/1.1\r\n"     
		"Host: %s\r\n"   
		"Accept: */*\r\n"
		"Accept-Language: zh-CN\r\n"
		"Accept-Encoding: gzip, deflate\r\n"
		"User-Agent: %s\r\n"   
		"Pragma: no-cache\r\n"   
		"Cache-Control: no-cache\r\n"   
		"Connection: Keep-Alive\r\n";
		//"Cookie: %s\r\n\r\n";
	CStringA szHeader;
	szHeader.Format(szRequestF, szTmpHostEnd ? szTmpHostEnd : "/", szHost, lpszUserAgent);
	if (lpszCookie) {
		szHeader.AppendFormat("Cookie: %s\r\n", lpszCookie);
	}
	szHeader += "\r\n";

	if (send(s, szHeader, szHeader.GetLength(), 0) == SOCKET_ERROR)
	{
		i_ret = 4;
		goto exit_l;
	}
	if (!RecvDataEx(s, pBuf, szheader))
	{
		i_ret = 5;
		goto exit_l;
	}

exit_l:
	//if (h) free(h);
	closesocket(s); 
	return i_ret;
}

int WinSocketDownload(LPCSTR lpszUrl, CBuffer *pBuf)
{
	return WinSocketDownloadEx(lpszUrl, "Snow/0.b2 (TengAttack App Engine/0.1)", NULL, pBuf, NULL);
}
