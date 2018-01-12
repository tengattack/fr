
#ifndef _TA_NET_BASE_H_
#define _TA_NET_BASE_H_ 1
#pragma once

#include <string>
#include <winsock2.h>
#include <windows.h>
#include <common/Buffer.h>

struct curl_slist;
typedef void CURL;

#if defined(_SNOW) || defined(_SNOW_COMMON_H_)
#include "G:\projects\Code\Snow\Snow\common.h"
#else
enum SnowProxyType{
	kHttp = 0,
	kSocks4,
	kSocks5
};
#endif

extern size_t CurlOnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid);
extern void HttpProcessContent(CBuffer& header, CBuffer& content);

/*extern bool RecvDataEx(SOCKET sock, CBuffer *buf, char **szHeader = NULL);
extern bool RecvDataEx(SOCKET sock, char **lpszData);*/
extern void SocketDefaultSet(SOCKET sock);
extern bool AnalyzeRecvData(CBuffer *inbuf, CBuffer *outbuf, char **szRetHeader);

#define TA_NET_OK	0

class CRecvDataMgr
{
public:
	CRecvDataMgr();
	~CRecvDataMgr();

	void Clear();

	char* FindHeaderEndFlags(unsigned char* buffer, int len);
	char* FindLineEndFlags(char* line, int len);
	bool CheckFinish(unsigned char* buffer, int len);
	
	CBuffer m_buffer;

protected:
	bool m_bfirst;
	bool m_bchunked;
	bool m_bfinished;
	int m_next_size;
	int m_headers_offset;
};


class CTANetBase
{
public:
	CTANetBase(void);
	virtual ~CTANetBase(void);

	virtual void SetCookie(LPCSTR lpszCookie);
	virtual LPCSTR GetCookie();
	inline std::string& GetStringCookie()
	{
		return m_str_cookie_;
	};

	virtual const char* GetUrlPrefix();
	virtual const char* GetUserAgent();

//proxy
	inline void RemoveProxy()
	{
		m_use_proxy_ = false;
		m_proxy_str_address_.clear();
	};
	inline bool IsUseProxy()
	{
		return m_use_proxy_;
	};
	inline bool IsUseHttpProxy()
	{
		return (m_use_proxy_ && (m_proxy_type_ == kHttp));
	};
	
	bool InitProxySocket(SOCKET *pSock, sockaddr_in *ptr_server_addr = NULL, char *http_domain = NULL);
	bool SetProxy(LPCSTR lpszIP, DWORD nPort, SnowProxyType type = kHttp);

	int CurlRequestGet(LPCSTR url, CBuffer& response, CBuffer *header = NULL);
	int CurlRequestPost(LPCSTR url, LPCSTR postdata, CBuffer& response, CBuffer *header);
	int CurlRequestEx(bool ispost, CURL *curl, CBuffer& response, CBuffer *header = NULL);
  void CurlSetWriteFunction(CURL *curl, CBuffer& response, CBuffer *header);

protected:
	std::string m_str_cookie_;

	bool m_use_proxy_;
	std::string m_proxy_str_address_;
	struct sockaddr_in m_proxy_addr;
	SnowProxyType m_proxy_type_;

	bool InitHttpProxySocket(SOCKET *pSock);
	bool InitSocks4ProxySocket(SOCKET *pSock, sockaddr_in *ptr_server_addr = NULL, char *http_domain = NULL);
	bool InitSocks5ProxySocket(SOCKET *pSock, sockaddr_in *ptr_server_addr = NULL, char *http_domain = NULL);

	virtual curl_slist* post_addheader(curl_slist *list);
	virtual void post_setopt(CURL *curl);
	virtual curl_slist* get_addheader(curl_slist *list);
	virtual void get_setopt(CURL *curl);
	
public:
  virtual curl_slist* addheader(struct curl_slist *list);
	virtual void setopt(CURL *curl);
};

#endif