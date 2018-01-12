
#include <common/Buffer.h>

namespace taSSL{
	extern bool m_bInit;
	extern bool Init();
	extern void Uninit();
	extern int GetSSLData(SOCKET sock, const char *szHeader, const char *szPost,
		CBuffer *buf, char **szRetHeader = NULL);
	extern int GetSSLData(SOCKET sock, BYTE *dataHeader, int hdrLen, BYTE *dataPost, int ptLen,
		CBuffer *buf, char **szRetHeader = NULL);
	
};


//Error Code
#define TA_SSL_SUCCESS				0
#define TA_SSL_ERROR				-1
#define TA_SSL_NEW_FAILED			-2
#define TA_SSL_LINK_SOCKET_FAILED	-3
#define TA_SSL_CONNECT_FAILED		-4
#define TA_SSL_SEND_FAILED			-5
#define TA_SSL_RECV_FAILED			-6
#define TA_SSL_SHUTDOWN_FAILED		-7

//common
#define TA_SSL_BLOCK_LEN			4096