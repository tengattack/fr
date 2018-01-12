
#ifndef _NET_MAIL_21CN_H_
#define _NET_MAIL_21CN_H_ 1
#pragma once

#include <string>

#include "MailBase.h"

class CMail21cn : public CMailBase
{
public:
	CMail21cn();
	~CMail21cn();

	virtual const char* GetUserAgent();

	int Register(LPCSTR name, LPCSTR password, char** ret_info = NULL);
	int GetRegverifyStr(LPCSTR name, LPCSTR password, std::string& regverifyStr);

protected:

	void SetWACookie();
	virtual curl_slist* addheader(struct curl_slist *list);
};

#endif