
#ifndef _TA_SCRIPT_TAV8_HTTP_H_
#define _TA_SCRIPT_TAV8_HTTP_H_

#pragma once

#include <vector>
#include <string>
#include <net/http/TANetBase.h>
#include "ScriptBase.h"

//-----------V8HTTP for V8Engine--------------

typedef struct _TA_REQUEST_HEADER {
	char *szHeader;
	char *szData;
} TA_REQUEST_HEADER;

class CTAV8Http : public CScriptBase, public CTANetBase
{
public:
	CTAV8Http(void);
	virtual ~CTAV8Http(void);

	void Clean();
	bool Open(LPCSTR lpszMethod, LPCSTR lpszUrl);
	void SetRequestHeader(LPCSTR lpszHeader, LPCSTR lpszData);
	bool Send(LPCSTR lpszData = NULL);
	bool SyncSend(LPCSTR lpszData = NULL);

	char *szResponseText;
	char *szResponseHeader;

	virtual int SyncWork();
	virtual void SyncFinish();

	void SetHost(LPCSTR host);
	bool InitSocket(SOCKET *sock);

	inline static void SetDefaultHost(LPCSTR host_) {
		m_def_host = host_;
	}

protected:
	std::string szRequest;
	std::vector<TA_REQUEST_HEADER> m_vecRequestHeader;

	static std::string m_def_host;

	bool m_sync_data_;
	std::string m_sync_send_;

	std::string m_str_host_;

	std::string m_str_method_;
	std::string m_str_url_;

	bool m_ssl_;
};

#endif //_TA_SCRIPT_TAV8_HTTP_H_