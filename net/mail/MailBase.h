
#ifndef _NET_MAIL_BASE_H_
#define _NET_MAIL_BASE_H_ 1
#pragma once

#include <net/baidu/BaiduBase.h>
#include <net/http/TACookiesManger.h>

#define GSASL_STATIC

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <vmime/types.hpp>

namespace vmime {
namespace net {
	class store;
}
}

class CMailBase : public CBaiduBase
{
public:
	CMailBase();
	virtual ~CMailBase();

	static bool staticInit();
	static void staticUninit();

	bool mailConnect(const char *protocolUrl, vmime::ref<vmime::net::store>& st);

	int getBaiduRegverifyStr(LPCSTR name, LPCSTR domain, LPCSTR password, std::string& regverifyStr);

	CTACookiesManger m_taCMgr;
};

#endif