
#include "StdAfx.h"
#include "ScriptTAV8Http.h"

#include <common/Buffer.h>
#include <common/strconv.h>

#include <base/string/stringprintf.h>
#include <net/http/taSSL.h>

#define TA_USERAGENT "Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.21 (KHTML, like Gecko) Chrome/25.0.1354.0 Safari/537.21"

extern bool RecvDataEx(SOCKET sock, CBuffer *buf, char **szHeader = NULL);

std::string CTAV8Http::m_def_host = "www.google.com";

#define _TA_DEF_HOST (m_def_host.c_str())

#define FREE_POINTER(p) \
					if (p) { \
						free(p); \
						p = NULL; \
					}

CTAV8Http::CTAV8Http(void)
	: m_sync_data_(false)
	, m_ssl_(false)
	, szResponseText(NULL)
	, szResponseHeader(NULL)
	, szRequest("")
	, m_str_host_(_TA_DEF_HOST)
{
}


CTAV8Http::~CTAV8Http(void)
{
	Clean();
}

void CTAV8Http::Clean()
{
	if (m_sync_data_)
	{
		m_sync_send_.clear();
		m_sync_data_ = false;
	}

	if (m_str_host_ != _TA_DEF_HOST)
		m_str_host_ = _TA_DEF_HOST;

	m_str_method_.clear();
	m_str_url_.clear();

	RemoveProxy();
	SetCookie(NULL);

	szRequest.clear();

	FREE_POINTER(szResponseText);
	FREE_POINTER(szResponseHeader);

	int c = m_vecRequestHeader.size();
	for (int i = 0; i<c ; i++)
	{
		if (m_vecRequestHeader[i].szHeader) free(m_vecRequestHeader[i].szHeader);
		if (m_vecRequestHeader[i].szData) free(m_vecRequestHeader[i].szData);
	}

	m_vecRequestHeader.clear();
}


bool CTAV8Http::Open(LPCSTR lpszMethod, LPCSTR lpszUrl)
{
	m_str_method_ = lpszMethod;
	m_str_url_ = lpszUrl;

	m_ssl_ = false;

	TA_REQUEST_HEADER rh = {0};
	//SetHost
	/*rh.szHeader = _strdup("Host");
	rh.szData = _strdup(_TA_DEF_HOST);
	m_vecRequestHeader.push_back(rh);*/


	std::string host_;
	if (::_strnicmp(m_str_url_.c_str(), "http://", 7) == 0) {
		host_ = m_str_url_.c_str() + 7;
	} else if (::_strnicmp(m_str_url_.c_str(), "https://", 8) == 0) {
		m_ssl_ = true;
		host_ = m_str_url_.c_str() + 8;
	} else {
		host_ = m_str_url_;
	}

	std::string uri_ = "/";
	char *endhost = strstr((char *)host_.c_str(), "/");
	if (endhost) {
		if (endhost[1]) uri_ = endhost;
		endhost[0] = 0;
	}

	if (strlen(host_.c_str()) > 0) { 
		SetHost(host_.c_str());
	}

	m_str_url_ = uri_;
	base::SStringPrintf(&szRequest, "%s %s HTTP/1.1\r\n", lpszMethod, m_str_url_.c_str());
	
	//"Accept: */*\r\n"
	rh.szHeader = _strdup("Accept");
	rh.szData = _strdup("*/*");
	m_vecRequestHeader.push_back(rh);

	//"Accept-Encoding: \r\n"
	rh.szHeader = _strdup("Accept-Encoding");
	rh.szData = _strdup("gzip, deflate");
	m_vecRequestHeader.push_back(rh);

	//"User-Agent: %TA_USERAGENT\r\n"
	rh.szHeader = _strdup("User-Agent");
	rh.szData = _strdup(TA_USERAGENT);
	m_vecRequestHeader.push_back(rh);

	if (lstrcmpA(lpszMethod, "POST") == 0)
	{
		//	"Content-Type: application/x-www-form-urlencoded\r\n"
		rh.szHeader = _strdup("Content-Type");
		rh.szData = _strdup("application/x-www-form-urlencoded");
		m_vecRequestHeader.push_back(rh);
	}

	rh.szHeader = _strdup("Connection");
	rh.szData = _strdup("Keep-Alive");
	m_vecRequestHeader.push_back(rh);

	return true;
}

void CTAV8Http::SetRequestHeader(LPCSTR lpszHeader, LPCSTR lpszData)
{
	//设置Cookie, Host
	if (_stricmp(lpszHeader, "cookie") == 0)
	{
		SetCookie(lpszData);
		return;
	} else if (_stricmp(lpszHeader, "host") == 0) {
		SetHost(lpszData);
		return;
	}

	int c = m_vecRequestHeader.size();
	bool bfind = false;
	for (int i = 0; i<c ; i++)
	{
		if (lstrcmpA(m_vecRequestHeader[i].szHeader, lpszHeader) == 0)
		{
			bfind = true;
			if (m_vecRequestHeader[i].szData) free(m_vecRequestHeader[i].szData);

			m_vecRequestHeader[i].szData = _strdup(lpszData);
		}
	}
	if (!bfind)
	{
		TA_REQUEST_HEADER rh = {0};
		rh.szHeader = _strdup(lpszHeader);
		rh.szData = _strdup(lpszData);
		m_vecRequestHeader.push_back(rh);
	}
}

void CTAV8Http::SetHost(LPCSTR host)
{
	m_str_host_ = host;
}

bool CTAV8Http::SyncSend(LPCSTR lpszData)
{
	if (lpszData != NULL)
	{
		m_sync_data_ = true;
		m_sync_send_ = lpszData;
	}

	bool b = AddToSyncList();
	if (b) SyncStart();
	return b;
}

int CTAV8Http::SyncWork()
{
	//AutoLock autolock(m_lock);

	bool b_ret = false;
	if (m_sync_data_)
	{
		b_ret = Send(m_sync_send_.c_str());
	} else {
		b_ret = Send();
	}

	SetSyncResult(b_ret ? 1 : 0);
	SyncFinish();
	return 0;
}

void CTAV8Http::SyncFinish()
{
	CScriptBase::SyncFinish();

	if (m_sync_data_)
	{
		m_sync_send_.clear();
		m_sync_data_ = false;
	}
}

bool CTAV8Http::Send(LPCSTR lpszData)
{
	SOCKET sock = NULL;
	bool b_ret = false;
	bool b_find_content_length = false;

	if (!InitSocket(&sock))
		return false;

	if (IsUseHttpProxy())
	{
		//HTTP代理
		/*if (::_strnicmp(m_str_url_.c_str(), "http://", 7) != 0
			// || ::_strnicmp(m_str_url_.c_str(), "https://", 8) != 0
			 )*/
		{
			base::SStringPrintf(&szRequest, "%s http%s://%s%s HTTP/1.1\r\n", 
				m_str_method_.c_str(), 
				(m_ssl_ ? "s" : ""),
				m_str_host_.c_str(), m_str_url_.c_str());
		}
	}

	//建立数据头
	std::string szHeader(szRequest);
	CBuffer buf;

	//Host
	base::StringAppendF(&szHeader, "Host: %s\r\n", m_str_host_.c_str());
	//szHeader.AppendFormat(
	
	int c = m_vecRequestHeader.size();
	for (int i = 0; i<c ; i++)
	{
		base::StringAppendF(&szHeader, "%s: %s\r\n", m_vecRequestHeader[i].szHeader, m_vecRequestHeader[i].szData);
		if (lpszData)
		{
			if (lstrcmpA(m_vecRequestHeader[i].szHeader, "Content-Length") == 0)
			{
				b_find_content_length = true;
			}
		}
	}

	if (lpszData && !b_find_content_length)
	{
		base::StringAppendF(&szHeader, "Content-Length: %d\r\n", lstrlenA(lpszData));
	}

	if (m_str_cookie_.length() > 0)
	{
		base::StringAppendF(&szHeader, "Cookie: %s\r\n", GetCookie());
	}

	szHeader += "\r\n";

	FREE_POINTER(szResponseText);
	FREE_POINTER(szResponseHeader);

	if (m_ssl_) {
		if (taSSL::GetSSLData(sock, szHeader.c_str(), lpszData, &buf, &szResponseHeader) != TA_SSL_SUCCESS) {
			goto exit_l;
		}
	} else {
		if( SOCKET_ERROR == send(sock, szHeader.c_str(), szHeader.length(), 0) ) 
		{
			goto exit_l;
		}
		if (lpszData)
		{
			if( SOCKET_ERROR == send(sock, lpszData, lstrlenA(lpszData), 0) ) 
			{
				goto exit_l;
			}
		}
		if (!RecvDataEx(sock, &buf, &szResponseHeader))
		{
			goto exit_l;
		}
	}

	buf.WriteZeroByte();
	szResponseText = (char *)buf.GetBuffer();
	buf.SetDestoryFree(false);

	/*
	szResponseText = (char *)malloc(buf.GetBufferLen() + 1);
	memcpy(szResponseText, buf.GetBuffer(), buf.GetBufferLen());
	szResponseText[buf.GetBufferLen()] = 0;
	*/

	b_ret = true;

exit_l:
	closesocket(sock);
	return b_ret;
}

bool CTAV8Http::InitSocket(SOCKET *sock)
{
	///创建SOCKET对象,这样多线程下也可以使用了
	struct sockaddr_in destaddr;

	/*if (m_str_host_ == _TA_DEF_HOST)
	{
		memcpy(&destaddr, &CBaiduHelper::m_destaddr, sizeof(destaddr));
	} else */{

		const char* port = strstr(m_str_host_.c_str(), ":");
		char *host = (char *)m_str_host_.c_str();
		u_short nport = 80;
		if (m_ssl_) {
			//SSL
			nport = 443;
		}
		if (port) {
			port += 1;
			nport = atoi(port);

			host = _strdup(m_str_host_.c_str());
			char *_port = strstr(host, ":");
			if (_port) _port[0] = 0;
		}

		hostent* pHostEnt = gethostbyname(host);   
		if(pHostEnt == NULL)   
			return false;

		struct in_addr ip_addr;
		memcpy(&ip_addr, pHostEnt->h_addr_list[0], 4);///h_addr_list[0]里4个字节,每个字节8位 

		memset((void *)&destaddr, 0, sizeof(destaddr));
		destaddr.sin_family = AF_INET;
		destaddr.sin_port = htons(nport);
		destaddr.sin_addr = ip_addr;

		if (port) free(host);
	}

	if (IsUseProxy())
	{
		//return InitProxySocket(sock, NULL, _TA_DEF_HOST);
		return InitProxySocket(sock, &destaddr, NULL);
	} else {
		SOCKET r_sock = NULL;
		r_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);   
		if(r_sock == INVALID_SOCKET)   
			return false;  
 
		/*int nTime = 5000;
		setsockopt(r_sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTime, sizeof(nTime)); 
		setsockopt(r_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(nTime));*/
		SocketDefaultSet(r_sock);

		if( 0 != connect(r_sock, (struct sockaddr*)&destaddr, sizeof(destaddr)) ) 
			return false; 

		*sock = r_sock;
		return true;
	}
}