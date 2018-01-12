
#include "StdAfx.h"
#include "taSSL.h"

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#pragma comment(lib, "ssleay32.lib")
#pragma comment(lib, "libeay32.lib")

#include <common/Buffer.h>
#include <tcmalloc.h>

extern bool AnalyzeRecvData(CBuffer *inbuf, CBuffer *outbuf, char **szRetHeader);

#define DebugMsg

namespace taSSL{
	SSL_CTX *m_ctx;
	bool SSLRecvDataEx(SSL *ssl, CBuffer *buf, char **szRetHeader = NULL);
};

bool taSSL::m_bInit = false;

bool taSSL::Init()
{
    // SSL initialize
    SSL_library_init();
    SSL_load_error_strings();

    m_ctx = SSL_CTX_new(SSLv23_client_method());
    if (m_ctx == NULL) 
    {
        m_bInit = false;
	} else {
		m_bInit = true;
	}

    RAND_poll();
    while (RAND_status() == 0) 
    {
        unsigned short rand_ret = rand() % 65536;
        RAND_seed(&rand_ret, sizeof(rand_ret));
    }

	return m_bInit;
}

void taSSL::Uninit()
{
	if (m_bInit) {
		SSL_CTX_free(m_ctx);
		ERR_free_strings();

		m_bInit = false;
	}
}


bool taSSL::SSLRecvDataEx(SSL *ssl, CBuffer *buf, char **szRetHeader)
{
	if (!buf) return false;
	//if (!szScr) return false;

	BYTE buffer[TA_SSL_BLOCK_LEN];
	int nResult;

	CBuffer recvBuf;
	int i_again = 0;
	//http://www.openssl.org/docs/ssl/SSL_read.html

	while (true)
	{
		nResult = SSL_read(ssl, buffer, TA_SSL_BLOCK_LEN); //获取数据
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
		recvBuf.Write(buffer, nResult);
	}

	return AnalyzeRecvData(&recvBuf, buf, szRetHeader);
}

//重载
int taSSL::GetSSLData(SOCKET sock, const char *szHeader, const char *szPost, CBuffer *buf, char **szRetHeader)
{
	return GetSSLData(sock, (BYTE *)szHeader, lstrlenA(szHeader), (BYTE *)szPost, lstrlenA(szPost), buf, szRetHeader);
}

int taSSL::GetSSLData(SOCKET sock, BYTE *dataHeader, int hdrLen, BYTE *dataPost, int ptLen, CBuffer *buf, char **szRetHeader)
{
	int i_ret = TA_SSL_ERROR;

    SSL *ssl;

    ssl = SSL_new(m_ctx);
    if (ssl == NULL) 
    {
        return TA_SSL_NEW_FAILED;
    }
    
    // link socket & SSL
    int ret = SSL_set_fd(ssl, sock);
    if (ret == 0) 
    {
        return TA_SSL_LINK_SOCKET_FAILED;
    }
    
    // SSL connect
    ret = SSL_connect(ssl);
    if (ret != 1) 
    {
        return TA_SSL_CONNECT_FAILED;
    }
    
    // send https request

    int totalsend = 0;
    int requestLen = hdrLen;
    while (totalsend < requestLen) 
    {
        int send = SSL_write(ssl, dataHeader + totalsend, requestLen - totalsend);
        if (send == -1) 
        {
            return TA_SSL_SEND_FAILED;
        }
        totalsend += send;
    }

	if (dataPost)
	{
		totalsend = 0;
		requestLen = ptLen;
		while (totalsend < requestLen) 
		{
			int send = SSL_write(ssl, dataPost + totalsend, requestLen - totalsend);
			if (send == -1) 
			{
				return TA_SSL_SEND_FAILED;
			}
			totalsend += send;
		}
	}


    // receive https response
    /*int responseBlockLen = 2048;
    int totalrev = 0;

    char returnBuffer[2049];
    //memset(returnBuffer, 0, 1025);
	while(true)
	{
		totalrev = 0;
		int rev = 0;
		while (totalrev < responseBlockLen) 
		{
			rev = SSL_read(ssl, returnBuffer + totalrev, responseBlockLen - totalrev);
			if (rev == 0)
			{
				//rev end.
				break;
			}
			if (rev == -1) 
			{
				i_ret = TA_SSL_RECV_FAILED;
				goto exit_l;
			}
			totalrev += rev;
			//DebugMsg("%d bytes rev OK!\r\n", totalrev);
		}
		returnBuffer[totalrev] = 0;

		if (rev == 0) break;
	}*/

	//Sleep(200);
	if (SSLRecvDataEx(ssl, buf, szRetHeader))
		i_ret = TA_SSL_SUCCESS;
	else
		i_ret = TA_SSL_RECV_FAILED;

exit_l:

    // shutdown community 
    ret = SSL_shutdown(ssl);
    if (ret != 1)
    {
		//TA_SSL_SHUTDOWN_FAILED
        //DebugMsg("SSL shutdown failed!\r\n");
        if (i_ret != TA_SSL_SUCCESS) return TA_SSL_SHUTDOWN_FAILED;
    }
    SSL_free(ssl);

    return i_ret;
}