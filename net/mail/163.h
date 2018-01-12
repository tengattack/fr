
#ifndef _NET_MAIL_163_H_
#define _NET_MAIL_163_H_ 1
#pragma once

#include <string>

#include "MailBase.h"

class CMail163 : public CMailBase
{
public:
	CMail163();
	~CMail163();

	virtual const char* GetUserAgent();

	int Register(LPCSTR name, LPCSTR password, char** ret_info = NULL);
	int GetRegverifyStr(LPCSTR name, LPCSTR password, std::string& regverifyStr);

protected:
	std::string GetEnvValue(const char* a);
};

#endif