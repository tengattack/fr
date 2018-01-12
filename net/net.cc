
#include "stdafx.h"

#include "net.h"
#include <zlibhelper.h>
#include <common/Buffer.h>

#include "http/http_chunked_decoder.cc"
#include "http/TACookiesManger.cpp"
#include "http/TANetBase.cpp"
#include "http/taSSL.cpp"

namespace net {
	bool Init()
	{
		WSADATA   wsaData;   
		if(WSAStartup(MAKEWORD(2,1), &wsaData) || LOBYTE(wsaData.wVersion) != 2) {
			return false;
		} else {
			taSSL::Init();
			return true;
		}
	}

	void Uninit()
	{
		taSSL::Uninit();
		WSACleanup();
	}

}

#define segment_size 1460//largest tcp data segment

int ungzip(char* source, int len, CBuffer& buf)
{
	int ret,have;
	//int offset = 0;
	
	z_stream d_stream = {0};	//≥ı ºªØz_stream
	Byte uncompr[segment_size * 4];
	
	uLong comprLen, uncomprLen;

	comprLen = len;
	uncomprLen = segment_size * 4;
	
	ret = inflateInit2(&d_stream, 47);
	if(ret != Z_OK) {
	   //printf("inflateInit2 error: %d", ret);
	   return ret;
	}

	d_stream.next_in = (Byte *)source;
	d_stream.avail_in = comprLen;
	do
	{
		d_stream.next_out = uncompr;
		d_stream.avail_out = uncomprLen;
		ret = inflate(&d_stream, Z_NO_FLUSH);
		switch (ret)
		{
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;   
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			inflateEnd(&d_stream);
		case Z_STREAM_ERROR:
			return ret;
		}
		have = uncomprLen - d_stream.avail_out;
		buf.Write(uncompr, have);

		//memcpy(des + offset, uncompr, have);
		//offset += have;
	} while (d_stream.avail_out == 0);

	inflateEnd(&d_stream);
	//des[offset] = 0;
	//memcpy(des+offset,"\0",1);
	if (ret == Z_STREAM_END) {
		ret = Z_OK;
	}

	return ret;
}
 