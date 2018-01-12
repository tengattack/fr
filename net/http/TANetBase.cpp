
#include "stdafx.h"

#include "TANetBase.h"

#include <tcmalloc.h>

#include <base/string/stringprintf.h>
#include <common/Buffer.h>

#include <net/net.h>
#include <curlhelper.h>

//gzip
#ifndef _TA_NOGZIP
#include <zlibhelper.h>
#endif

//ms
#define DEFAULT_TIME_OUT	5000

#ifdef _SNOW
namespace snow {
	namespace config {
		extern int timeout;
	};
};
#undef DEFAULT_TIME_OUT
#define DEFAULT_TIME_OUT   (snow::config::timeout)
#endif

#define _TA_RETRY_TIMES		50
#define TA_BLOCK_LEN		4096

size_t CurlOnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
  CBuffer* pbuf = (CBuffer *)lpVoid;  
  if(NULL == pbuf || NULL == buffer) {
      return -1;
  }
  
  pbuf->Write((unsigned char *)buffer, size * nmemb);

  return nmemb;
}

void HttpProcessContent(CBuffer& header, CBuffer& content)
{
	bool bIsChunked = false, bIsGzip = false;
	char* chheader = reinterpret_cast<char *>(header.GetBuffer());
	char* tstr;

	if (strstr(chheader, "chunked"))
	{
		bIsChunked = true;
	} else {
		bIsChunked = false;
	}

	//判断是否为gzip的
	tstr = strstr(chheader, "Content-Encoding:");
	if (tstr)
	{
		tstr = strstr(tstr, "gzip");
		if (tstr) 
		{
			//buf = &cBuf;
			bIsGzip = true;
		}
	}

	//curl会处理chunked
	/*if (bIsChunked) {
		net::HttpChunkedDecoder cd;
		int len = cd.FilterBuf((char *)content.GetBuffer(), content.GetBufferLen());
		content.SetBufferLen(len);
	}*/

	if (bIsGzip) {
		CBuffer buf;
		ungzip((char *)content.GetBuffer(), content.GetBufferLen(), buf);

		content.SetBufferLink(buf.GetBuffer(), buf.GetBufferLen(), buf.GetMemSize());
		buf.SetDestoryFree(false);
	}
}

//CRecvDataMgr
CRecvDataMgr::CRecvDataMgr()
{
	Clear();
}

CRecvDataMgr::~CRecvDataMgr()
{
}

void CRecvDataMgr::Clear()
{
	m_bfirst = true;
	m_bchunked = false;
	m_next_size = 0;
	m_headers_offset = 0;
	m_bfinished = false;
	m_buffer.ClearBuffer();
}

char* CRecvDataMgr::FindHeaderEndFlags(unsigned char* buffer, int len)
{
	for (int i = 0; i < len - 4; i++)
	{
		if (memcmp(buffer + i, "\r\n\r\n", 4) == 0)
		{
			return reinterpret_cast<char *>(buffer + i);
			break;
		}
	}
	return NULL;
}

char* CRecvDataMgr::FindLineEndFlags(char* line, int len)
{
	for (int i = 0; i < len - 2; i++)
	{
		if (memcmp(line + i, "\r\n", 2) == 0)
		{
			return line + i;
			break;
		}
	}
	return NULL;
}

bool CRecvDataMgr::CheckFinish(unsigned char* buffer, int len)
{
	if (len > 0)
	{
		m_buffer.Write(buffer, len);

		if (m_bfirst)
		{
			char* pEnd = FindHeaderEndFlags(m_buffer.GetBuffer(), m_buffer.GetBufferLen());
			if (pEnd)
			{
				m_bfirst = false;

				pEnd[0] = 0;

				char* pBuffer = reinterpret_cast<char *>(m_buffer.GetBuffer());

				if (strstr(pBuffer, "chunked"))
				{
					m_bchunked = true;
				} else {
					m_bchunked = false;
				}

				pEnd[0] = '\r';

				m_next_size = pEnd + 4 - pBuffer;
				m_headers_offset = m_next_size;
			}
		}

		if (m_bchunked)
		{
			char* pBuffer = reinterpret_cast<char *>(m_buffer.GetBuffer() + m_next_size);
			int len = m_buffer.GetBufferLen() - m_next_size;
			int nChunkSize;
			char *nChunkData, *strLineEnd, *strLine = pBuffer;

			while (len > 0)
			{
				nChunkSize = strtoul(strLine, &strLineEnd, 16);
				if (nChunkSize == 0)
				{
					m_bfinished = true;
					break;
				}
				if (nChunkSize > len - (strLineEnd - strLine)) break;

				nChunkData = FindLineEndFlags(strLine, len - (strLineEnd - strLine));
				if (!nChunkData) break;
				nChunkData += 2;//'\r\n'

				strLine = nChunkData + nChunkSize + 2;	//'\r\n'
				len = m_buffer.GetBufferLen() - (strLine - reinterpret_cast<char *>(m_buffer.GetBuffer()));
				m_next_size = (strLine - reinterpret_cast<char *>(m_buffer.GetBuffer()));
			}
		}

		return m_bfinished;
	} else {
		return true;
	}
}

void SocketDefaultSet(SOCKET sock)
{
	int nTime = DEFAULT_TIME_OUT;

	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&nTime, sizeof(nTime)); 
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTime, sizeof(nTime));
}

bool AnalyzeRecvData(CBuffer *inbuf, CBuffer *outbuf, char **szRetHeader)
{
	if (!inbuf || !outbuf) return false;
	
	bool bIsChunked = false, bIsGzip = false;
	char *strLine = NULL;
	char *tstr = NULL;

	if (inbuf->GetBufferLen() > 0)
	{
		char* pBuffer = reinterpret_cast<char *>(inbuf->GetBuffer());
		int len = inbuf->GetBufferLen();

		if (strLine == NULL)
		{
			strLine = strstr(pBuffer, "\r\n\r\n");
			if (!strLine) return false;
			strLine[0] = 0;
			if (szRetHeader) *szRetHeader = _strdup(pBuffer);
			
			if (strstr(pBuffer, "chunked"))
			{
				bIsChunked = true;
			} else {
				bIsChunked = false;
			}

			//判断是否为gzip的
			tstr = strstr(pBuffer, "Content-Encoding:");
			if (tstr)
			{
				tstr = strstr(tstr, "gzip");
				if (tstr) 
				{
					//buf = &cBuf;
					bIsGzip = true;
				}
			}

			strLine[0] = '\r';
			strLine += 4;	//"\r\n\r\n"
		}
		
		len -= (strLine - pBuffer);
		if (len > 0)
		{
			if (bIsChunked)
			{
				net::HttpChunkedDecoder cd;
				len = cd.FilterBuf(strLine, len);
				if (len > 0)
				{
					outbuf->Write(reinterpret_cast<BYTE *>(strLine), len);
				}
			} else {
				outbuf->Write(reinterpret_cast<BYTE *>(strLine), len);
			}
		}
	}

	if (outbuf->GetBufferLen() <= 0) 
		return false;
	
	//char *szRetScr;
#ifndef _TA_NOGZIP
	if (bIsGzip)
	{
		CBuffer uncompressbuf;
		ungzip((char *)outbuf->GetBuffer(), outbuf->GetBufferLen(), uncompressbuf);

		if (uncompressbuf.GetBufferLen() > 0) {
			outbuf->SetBufferLink(uncompressbuf.GetBuffer(), 
				uncompressbuf.GetBufferLen(),
				uncompressbuf.GetMemSize());
			uncompressbuf.SetDestoryFree(false);
			outbuf->WriteZeroByte();	//字符中断
		}
	}
#endif
	/*else{
		szRetScr = (char *)malloc(outbuf->GetBufferLen() + 1);
		memcpy(szRetScr, outbuf->GetBuffer(), outbuf->GetBufferLen());
		szRetScr[outbuf->GetBufferLen()] = 0;
	}*/
	return true;
}

bool RecvDataEx(SOCKET sock, CBuffer *buf, char **szHeader)
{
	if (!buf) return false;

	char buffer[TA_BLOCK_LEN];
	int nResult;

	CRecvDataMgr rdm;
	int i_again = 0;
	/*
	If   no   error   occurs,   recv   returns   the   number   of   bytes   received.
	If   the   connection   has   been   gracefully   closed,   the   return   value   is   zero.
	Otherwise,   a   value   of   SOCKET_ERROR   is   returned,
		and   a   specific   error   code   can   be   retrieved   by   calling   WSAGetLastError. 
	*/
	
	while (true)
	{
		nResult = recv(sock, buffer, TA_BLOCK_LEN, 0); //获取数据

		//if (ioctlsocket(sock, SIOCATMARK)
		
		if (nResult <= 0)
		{
			break;
		}/* else if (nResult < 0) {
			//5次重试
			if (i_again < 5)
			{
				i_again++;
				Sleep(200);
				continue;
			} else {
				break;
			}
		}*/
		
		//检查是否完成
		if (rdm.CheckFinish(reinterpret_cast<unsigned char *>(buffer), nResult))
		{
			break;
		}
	}

	return AnalyzeRecvData(&rdm.m_buffer, buf, szHeader);
}

bool RecvDataEx(SOCKET sock, LPSTR *szScr)
{
	CBuffer buf;
	bool ret = RecvDataEx(sock, &buf);
	if (ret)
	{
		//以0结尾的字符串
		buf.WriteZeroByte();
		buf.SetDestoryFree(false); //销毁时不清空数据

		*szScr = reinterpret_cast<LPSTR>(buf.GetBuffer());
	}
	return ret;
}

int RecvData(SOCKET sock, LPCSTR request, LPSTR *szScr)
{
	//发送请求 
	if (send(sock, request, lstrlenA(request), 0) == SOCKET_ERROR)
	{
		return -2;
	}

	if (!RecvDataEx(sock, szScr))
	{
		return -4;
	}

	return 0;
}

CTANetBase::CTANetBase(void)
	: m_use_proxy_(false)
	, m_proxy_type_(kHttp)
{
}


CTANetBase::~CTANetBase(void)
{
}

bool CTANetBase::SetProxy(LPCSTR lpszIP, DWORD nPort, SnowProxyType type)
{
	//根据主机名获得IP地址   
	hostent* pHostEnt = gethostbyname(lpszIP);
	if (pHostEnt == NULL) {
		return false;
	}

	//连接 
	memset((void *)&m_proxy_addr, 0, sizeof(m_proxy_addr));   
	m_proxy_addr.sin_family = AF_INET; 
	m_proxy_addr.sin_port = htons(nPort); 
	memcpy(&m_proxy_addr.sin_addr.S_un.S_addr, pHostEnt->h_addr, pHostEnt->h_length);  

	m_use_proxy_ = true;
	m_proxy_type_ = type;

	//使用inet_ntoa可能不安全 TAMARK
	char *strip = inet_ntoa(m_proxy_addr.sin_addr);
	base::SStringPrintf(&m_proxy_str_address_, "%s:%u", strip, nPort);

	return true;
}

bool CTANetBase::InitHttpProxySocket(SOCKET *pSock)
{
	SOCKET r_sock;
	r_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);   
	if(r_sock == INVALID_SOCKET)
		return false;  
 
	//代理的延迟就高点吧
	SocketDefaultSet(r_sock);

	if( 0 != connect(r_sock, (struct sockaddr*)&m_proxy_addr, sizeof(m_proxy_addr)) ) 
		return false; 

	*pSock = r_sock;
	return true;
}

bool CTANetBase::InitSocks4ProxySocket(SOCKET *pSock, sockaddr_in *ptr_server_addr, char *http_domain)
{
//具体参见 http://zh.wikipedia.org/wiki/SOCKS

	SOCKET r_sock;
	if (!InitHttpProxySocket(&r_sock))
	{
		return false;
	}

	//var
	int nRcvd=0,nCount=0;

// Step 1: 下面是客户端向SOCKS 4代理服务器，发送的连接请求包的格式（以字节为单位）：
/*	+——+——+——+——+——+——+——+——+——+——+....+——+
	| VN | CD | DSTPORT |      DSTIP        | USERID       |NULL|
	+——+——+——+——+——+——+——+——+——+——+....+——+
	   1    1      2              4           variable       1		*/
//	VN是SOCK版本，应该是4；
//	CD是SOCK的命令码，1表示CONNECT请求，2表示BIND请求；
//	DSTPORT表示目的主机的端口；
//	DSTIP指目的主机的IP地址；
	//char reqNego[9]={'\x04','\x01', 0};
	char reqNego[9]={0};
	char resNego[8]={0};

	reqNego[0] = 4;
	reqNego[1] = 1;
	*(unsigned short *)(&reqNego[2]) = ptr_server_addr->sin_port;
	*(unsigned long *)(&reqNego[4]) = ptr_server_addr->sin_addr.S_un.S_addr;

	int nRet = send(r_sock, reqNego, 9, 0);
	if (nRet == SOCKET_ERROR) goto error_l;


//	代理服务器而后发送回应包（以字节为单位）：
/*	+——+——+——+——+——+——+——+——+
	| VN | CD | DSTPORT |      DSTIP        |
	+——+——+——+——+——+——+——+——+
	   1    1      2              4				*/
//	VN是回应码的版本，应该是0；
//	CD是代理服务器答复，有几种可能：
//	90，请求得到允许；
//	91，请求被拒绝或失败；
//	92，由于SOCKS服务器无法连接到客户端的identd（一个验证身份的进程），请求被拒绝；
//	93，由于客户端程序与identd报告的用户身份不同，连接被拒绝。
//	DSTPORT与DSTIP与请求包中的内容相同，但被忽略。
//	如果请求被拒绝，SOCKS服务器马上与客户端断开连接；如果请求被允许，代理服务器就充当客户端与目的主机之间进行双向传递，对客户端而言，就如同直接在与目的主机相连。

	
	nRcvd=0;
	nCount=0;
	while(true)
	{
		//接收sock[0]发送来的数据
		do{
			nRet = recv(r_sock, (char *)resNego + nRcvd, 8 - nRcvd,0);
			if(nRet==SOCKET_ERROR)
			{
				goto error_l;
			}
			nRcvd += nRet;
		}
		while((nRcvd < 8) && (++nCount < _TA_RETRY_TIMES));
		if(nRcvd >= 8) break;
		
		if(nCount++ >= _TA_RETRY_TIMES){
			goto error_l;
		}
	}

	if(resNego[0] != 0 || (resNego[1] != 90)){
		goto error_l;
	};

// 至此，连接已经建立。在此套接字上可进行数据的收发。

	*pSock = r_sock;
	return true;

error_l:
	closesocket(r_sock);
	return false;
}

bool CTANetBase::InitSocks5ProxySocket(SOCKET *pSock, sockaddr_in *ptr_server_addr, char *http_domain)
{
//具体参见 http://www.ietf.org/rfc/rfc1928.txt
	
	SOCKET r_sock;
	if (!InitHttpProxySocket(&r_sock))
	{
		return false;
	}

	//var
	fd_set fdread;
	int nRcvd=0,nCount=0;
	timeval tout;
	char resSubNego1[5]={'\0'};
	char resNego[2]={0};
	//5秒超时
	tout.tv_sec = (DEFAULT_TIME_OUT / 1000);
	tout.tv_usec = DEFAULT_TIME_OUT;

// Step 1: 连接代理服务器成功后，马上开始和代理协商，协商报文如下,询问服务器，版本5，是需要验证(0x02)还是不需要验证(0x00)
/*	+------+-------------------+------------+
　　|VER   | Number of METHODS | METHODS    |
　　+------+-------------------+------------+
　　| 0x05 | 0x02 (有两个方法) | 0x00 | 0x02|
　　+------+-------------------+------------+	*/
	const char reqNego[4]={'\x05','\x02','\x00','\x02'};
	int nRet = send(r_sock, reqNego, 4, 0);
	if (nRet == SOCKET_ERROR) goto error_l;

// Setp 2: 代理服务器将返回两个字节的协商结果，接收协商结果
	FD_ZERO(&fdread);
	FD_SET(r_sock,&fdread);

// Last param set to NULL for blocking operation. (struct timeval*)
	if((nRet = select(0,&fdread,NULL,NULL,&tout)) == SOCKET_ERROR)
	{
		goto error_l;
	}

	nRcvd=0;
	nCount=0;
	while(true)
	{
		if(FD_ISSET(r_sock, &fdread))
		{
			//接收sock[0]发送来的数据
			do{
				nRet = recv(r_sock, (char*)resNego + nRcvd, 2 - nRcvd,0);
				if(nRet==SOCKET_ERROR)
				{
					goto error_l;
				}
				nRcvd += nRet;
			}
			while((nRcvd < 2) && (++nCount < _TA_RETRY_TIMES));
			if(nRcvd >= 2) break;
		}
		if(nCount++ >= _TA_RETRY_TIMES){
			//return NC_E_PROXY_RECEIVE|WSAGetLastError();
			goto error_l;
		}
	}

	//验证是否成功
	if(resNego[0] != 0x05 || (resNego[1] != 0x00 && resNego[1] != 0x02))
	{
		//return NC_E_PROXY_PROTOCOL_VERSION|WSAGetLastError();
		goto error_l;
	}

// Step 3: 根据协商结果判断是否需要验证用户，如果是0x02，则需要提供验证，验证部分参考RFC1929
	if(resNego[1] == 0x02)
	{
		// 不支持
		/*
		// 需要密码验证
		char reqAuth[513]={'\0'};
		BYTE byLenUser = (BYTE)strlen(m_szProxyUserName);
		BYTE byLenPswd = (BYTE)strlen(m_szProxyPassword);
		reqAuth[0]=0x01;
		reqAuth[1]=byLenUser;
		sprintf(&reqAuth[2],"%s",m_szProxyUserName);
		reqAuth[2+byLenUser]=byLenPswd;
		sprintf(&reqAuth[3+byLenUser],"%s",m_szProxyPassword);
		//Send authentication info
		int len = (int)byLenUser + (int)byLenPswd + 3;
		nRet=send(m_socTCP,(const char*)reqAuth,len,0);
		if (nRet==SOCKET_ERROR){return NC_E_PROXY_SEND|WSAGetLastError();}
		//Now : Response to the auth request
		char resAuth[2]={'\0'};
		int nRcvd=0,nCount=0;
		do{
		nRet = recv(m_socTCP,resAuth+nRcvd,2-nRcvd,0);
		if(nRet==SOCKET_ERROR){return NC_E_PROXY_RECEIVE|WSAGetLastError();}
		nRcvd += nRet;
		}
		while((nRcvd!=2)&&(++nCount<1000));
		if(nCount>=1000){return NC_E_PROXY_RECEIVE|WSAGetLastError();}
		if (resAuth[1]!=0) return NC_E_PROXY_AUTHORIZE;
		// 密码验证通过了
		*/
		goto error_l;
	}

// Step 4: 协商完成，开始发送连接远程服务器请求,请求报文格式如下：
/*	+----+-----+-------+------+----------+----------+
　　|VER | CMD |　RSV　| ATYP | DST.ADDR | DST.PORT |
　　+----+-----+-------+------+----------+----------+
　　| 1　| 　1 | 0x00  | 　1　| Variable |    2     |
　　+----+-----+-------+------+----------+----------+	*/
// CMD==0x01 表示连接, ATYP==0x01表示采用IPV4格式地址，DST.ADDR是远程服务器地址，DST.PORT是远程服务器端口
// 如果需要接受外来连接，则需要在连接完成之后，发送CMD==0x02绑定请求，代理将为此请求绑定一个套接字接受外部连接

	if (ptr_server_addr)
	{
		char reqSubNego[10]={'\x05','\x01','\x00','\x01','\x00','\x00','\x00','\x00','\x00','\x00'};

		*(unsigned long *)(&reqSubNego[4]) = ptr_server_addr->sin_addr.S_un.S_addr;
		*(unsigned short *)(&reqSubNego[8]) = ptr_server_addr->sin_port;

		nRet = send(r_sock,(const char *)reqSubNego, 10, 0);
		if (nRet == SOCKET_ERROR)
		{
			//return NC_E_PROXY_SEND|WSAGetLastError();
			goto error_l;
		}
	} else if (http_domain){
		int _name_size = lstrlenA(http_domain);
		if (_name_size > 255 || _name_size <= 0) goto error_l;

		int size = 4 + 1 + _name_size + 2;
		char *preqSubNego = new char[size];
		memcpy(preqSubNego, "\x05\x01\x00\x03", 4);
		((unsigned char *)preqSubNego)[4] = (unsigned char)_name_size;
		lstrcpyA(preqSubNego + 5, http_domain);
		*(unsigned short *)(preqSubNego + size - 2) = ntohs(80);

		nRet = send(r_sock, (const char *)preqSubNego, size, 0);
		if (nRet == SOCKET_ERROR)
		{
			delete[] preqSubNego;
			//return NC_E_PROXY_SEND|WSAGetLastError();
			goto error_l;
		}

		delete[] preqSubNego;
	} else {
		//无目标主机
		goto error_l;
	}

// Step 5: 接收对请求的响应，响应包格式如下
/*	+----+-----+-------+------+----------+----------+
　　|VER | REP |　RSV　| ATYP | BND.ADDR | BND.PORT |
　　+----+-----+-------+------+----------+----------+
　　| 1　|　1　| 0x00  |　1   | Variable | 　　2 　 |
　　+----+-----+-------+------+----------+----------+	*/
// VER 必须是0x05, REP==0x00表示成功，ATYP==0x01表示地址是IPV4地址，BND.ADDR 是代理为连接远程服务器绑定的地址，BND.PORT是这个套接字的端口
	
	//char resSubNego1[5]={'\0'};
	if(FD_ISSET(r_sock, &fdread))
	{
		nRcvd=0;
		nCount=0;

		do{
			nRet = recv(r_sock, resSubNego1 + nRcvd, 5 - nRcvd, 0);
			if(nRet == SOCKET_ERROR)
			{
				//return NC_E_PROXY_RECEIVE|WSAGetLastError();
				goto error_l;
			}
			nRcvd += nRet;
		}
		while((nRcvd < 5) && (++nCount < _TA_RETRY_TIMES));
		if(nCount >= _TA_RETRY_TIMES)
		{
			//return NC_E_PROXY_RECEIVE|WSAGetLastError();
			goto error_l;
		}
		if(resSubNego1[0] != 0x05 || resSubNego1[1] != 0x00)
		{
			//return NC_E_PROXY_PROTOCOL_VERSION_SUB|WSAGetLastError();
			goto error_l;
		}

		// type
		switch(resSubNego1[3])
		{
		case 0x01:
		{
			// IP v4 前面已经获取了ip的第一个字节
			char resSubNego2[6] = {resSubNego1[4], 0};
			if(FD_ISSET(r_sock, &fdread))
			{
				nRcvd=0;
				nCount=0;

				do{
					int nRet = recv(r_sock, &resSubNego2[1] + nRcvd, 5 - nRcvd, 0);
					if(nRet==SOCKET_ERROR)
					{
						//return NC_E_PROXY_RECEIVE|WSAGetLastError();
						goto error_l;
					}
					nRcvd += nRet;
				}
				while((nRcvd < 5) && (++nCount < _TA_RETRY_TIMES));
				if(nCount >= _TA_RETRY_TIMES)
				{
					//return NC_E_PROXY_RECEIVE|WSAGetLastError();
					goto error_l;
				}
			}

			/*
			// 得到代理绑定地址
			unsigned long  ulBINDAddr = *(unsigned long*)resSubNego2; // SOCKS BIND ADDR
			unsigned short usBINDPort = *(unsigned short*)&resSubNego2[4]; // SOCKS BIND PORT
			printf("%u.%u.%u.%u:%d\r\n", (unsigned char)((unsigned char *)&ulBINDAddr)[0],
				(unsigned char)((unsigned char *)&ulBINDAddr)[1],
				(unsigned char)((unsigned char *)&ulBINDAddr)[2],
				(unsigned char)((unsigned char *)&ulBINDAddr)[3], ntohs(usBINDPort));

			m_saiProxyBindTCP.sin_addr.S_un.S_addr=ulBINDAddr;
			m_saiProxyBindTCP.sin_port=usBINDPort;
			// 得到本机绑定地址
			int len = sizeof(m_saiHostTCP);
			getsockname(m_socTCP,(SOCKADDR*)&m_saiHostTCP,&len);

			printf("%u.%u.%u.%u:%d\r\n", (unsigned char)((unsigned char *)&m_saiHostTCP.sin_addr.S_un.S_addr)[0],
				(unsigned char)((unsigned char *)&m_saiHostTCP.sin_addr.S_un.S_addr)[1],
				(unsigned char)((unsigned char *)&m_saiHostTCP.sin_addr.S_un.S_addr)[2],
				(unsigned char)((unsigned char *)&m_saiHostTCP.sin_addr.S_un.S_addr)[3], ntohs(m_saiHostTCP.sin_port));
			*/
		}
			break;
		case 0x03:
		{
			// Domain name
			int nLen = resSubNego1[4] + 2;
			char* presSubNego2 = new char[nLen];
			if(FD_ISSET(r_sock, &fdread))
			{
				nRet= SOCKET_ERROR;
				nRcvd=0;
				nCount=0;

				do{
					nRet = recv(r_sock, presSubNego2 + nRcvd, nLen - nRcvd, 0);
					if(nRet == SOCKET_ERROR)
					{
						//return NC_E_PROXY_RECEIVE|WSAGetLastError();
						goto error_l;
					}
					nRcvd += nRet;
				}
				while((nRcvd < nLen)&&(++nCount < _TA_RETRY_TIMES));
				if(nCount >= _TA_RETRY_TIMES)
				{
					//return NC_E_PROXY_RECEIVE|WSAGetLastError();
					goto error_l;
				}
			}

			// 得到了其实毫无意义
			// 但是是必须的
			// 此时得到的是远程主机的Domain Name

			//unsigned short usBINDPort = *(unsigned short*)(presSubNego2+nLen-2); // BIND PORT;

			delete[] presSubNego2;
			presSubNego2=NULL;
		}
			break;
		case 0x04:
		{
		// IP v6 前面已经获取了ip的第一个字节
			char resSubNego2[18] = {resSubNego1[4], 0};
			if(FD_ISSET(r_sock, &fdread))
			{
				nRcvd=0;
				nCount=0;

				do{
					int nRet = recv(r_sock, &resSubNego2[1] + nRcvd, 17 - nRcvd, 0);
					if(nRet==SOCKET_ERROR)
					{
						//return NC_E_PROXY_RECEIVE|WSAGetLastError();
						goto error_l;
					}
					nRcvd += nRet;
				}
				while((nRcvd < 17) && (++nCount < _TA_RETRY_TIMES));
				if(nCount >= _TA_RETRY_TIMES)
				{
					//return NC_E_PROXY_RECEIVE|WSAGetLastError();
					goto error_l;
				}
			}

		//获取数据完后不用理
		}
			break;
		default:
		
			//奇怪的事情
			goto error_l;
			break;
		}	// end of switch
	}
// 至此，连接已经建立。在此套接字上可进行数据的收发。

	*pSock = r_sock;
	return true;

error_l:
	closesocket(r_sock);
	return false;
}

bool CTANetBase::InitProxySocket(SOCKET *pSock, sockaddr_in *ptr_server_addr, char *http_domain)
{
	if (!m_use_proxy_) return false;
	if (!pSock) return false;

	switch (m_proxy_type_)
	{
	case kHttp:
		return InitHttpProxySocket(pSock);
		break;
	case kSocks4:
		if (ptr_server_addr)
			return InitSocks4ProxySocket(pSock, ptr_server_addr, NULL);
		else if (http_domain)
			return InitSocks4ProxySocket(pSock, NULL, http_domain);

		break;
	case kSocks5:
		if (ptr_server_addr)
			return InitSocks5ProxySocket(pSock, ptr_server_addr, NULL);
		else if (http_domain)
			return InitSocks5ProxySocket(pSock, NULL, http_domain);

		break;
	}

	return false;
}

void CTANetBase::SetCookie(LPCSTR lpszCookie)
{
	if (lpszCookie == NULL)
	{
		m_str_cookie_ = "";
		return;
	}

	if (strncmp(lpszCookie, "Cookie:", 7) != 0)
	{
		m_str_cookie_ = lpszCookie;
	} else {
		if (lpszCookie[7] == ' ')
			m_str_cookie_ = lpszCookie + 8;
		else
			m_str_cookie_ = lpszCookie + 7;
	}
}

LPCSTR CTANetBase::GetCookie()
{
	return m_str_cookie_.c_str();
}

const char* CTANetBase::GetUrlPrefix()
{
	return NULL;
}

const char* CTANetBase::GetUserAgent()
{
	return NULL;
}

//curl function api

curl_slist* CTANetBase::post_addheader(curl_slist *list)
{
	return addheader(list);
}

void CTANetBase::post_setopt(CURL *curl)
{
	setopt(curl);
}

curl_slist* CTANetBase::get_addheader(curl_slist *list)
{
	return addheader(list);
}

void CTANetBase::get_setopt(CURL *curl)
{
	setopt(curl);
}

curl_slist* CTANetBase::addheader(curl_slist *list)
{
	list = curl_slist_append(list, "Expect:");	// removing the Expect:100 content type
	return list;
}

void CTANetBase::setopt(CURL *curl)
{
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	if (GetUserAgent()) {
		curl_easy_setopt(curl, CURLOPT_USERAGENT, GetUserAgent());
	}
	curl_easy_setopt(curl, CURLOPT_COOKIE, GetCookie());

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, DEFAULT_TIME_OUT);

	if (IsUseProxy()) {

		curl_easy_setopt(curl, CURLOPT_PROXY, m_proxy_str_address_.c_str());

		switch (m_proxy_type_) {
		case kHttp:
			//curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, true);
			curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			break;
		case kSocks4:
			curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			break;
		case kSocks5:
			curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
			break;
		}
		
	}
}

int CTANetBase::CurlRequestGet(LPCSTR url, CBuffer& response, CBuffer *header)
{
	CURL *curl;
	
	response.ClearBuffer();

	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);

		return CurlRequestEx(false, curl, response, header);
	}

	return -1;
}

int CTANetBase::CurlRequestPost(LPCSTR url, LPCSTR postdata, CBuffer& response, CBuffer *header)
{
	CURL *curl;

	CBuffer bheader;

	response.ClearBuffer();

	curl = curl_easy_init();
		
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);

		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);

		//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
		//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

		return CurlRequestEx(true, curl, response, header);
	}

	return -1;
}

void CTANetBase::CurlSetWriteFunction(CURL *curl, CBuffer& response, CBuffer *header)
{
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlOnWriteData);
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, CurlOnWriteData);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)header);
}

int CTANetBase::CurlRequestEx(bool ispost, CURL *curl, CBuffer& response, CBuffer *header)
{
	int ret = -1;

	CURLcode retcode;
	CBuffer bheader;
	struct curl_slist *headerlist = NULL;

	if (curl)
	{
		if (ispost) {
			headerlist = post_addheader(headerlist);
		} else {
			headerlist = get_addheader(headerlist);
		}

    CurlSetWriteFunction(curl, response, &bheader);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		//curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

		if (ispost) {
			post_setopt(curl);
		} else {
			get_setopt(curl);
		}

		//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);  
		//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

		retcode = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		curl_slist_free_all(headerlist);

		if (retcode != CURLE_OK) {
			ret = retcode;
			return ret;
		}
	}

	bheader.WriteZeroByte();
	HttpProcessContent(bheader, response);

	if (header) {
		header->SetBufferLink(bheader.GetBuffer(), bheader.GetBufferLen(), bheader.GetMemSize());
		bheader.SetDestoryFree(false);
	}

	ret = TA_NET_OK;

	return ret;
}
