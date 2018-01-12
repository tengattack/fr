
#ifndef _SNOW_CUTE_NET_H_
#define _SNOW_CUTE_NET_H_ 1

#include <winsock2.h>
#include <net/http/http_chunked_decoder.h>
#pragma comment(lib, "ws2_32.lib")

class CBuffer;

namespace net {
	bool Init();
	void Uninit();
}

extern int RecvData(SOCKET sock, LPCSTR request, LPSTR *szScr);
extern bool RecvDataEx(SOCKET sock, char **lpszData);
extern bool RecvDataEx(SOCKET sock, CBuffer *buf, char **szHeader = NULL);

int ungzip(char* source, int len, CBuffer& buf);

#endif