
#include "stdafx.h"
#include "21cn.h"

#include <string>
#include <time.h>
#include <net/net.h>

#include <common/strconv.h>
#include <common/Urlcode.h>

#include <base/string/stringprintf.h>
#include <base/string/string_number_conversions.h>
#include <base/rand_util.h>

#include <base/json/values.h>
#include <base/json/json_reader.h>

#include <curl/curl.h>
#include <vmime/vmime.hpp>

CMail21cn::CMail21cn()
{
}

CMail21cn::~CMail21cn()
{
}


curl_slist* CMail21cn::addheader(struct curl_slist *list)
{
	list = CMailBase::addheader(list);
	list = curl_slist_append(list, "Referer: http://fmail.21cn.com/freeinterface/jsp/register/reg_moblie.htm");
	return list;
}

const char* CMail21cn::GetUserAgent()
{
	return "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.14 Safari/537.36";
}

int CMail21cn::GetRegverifyStr(LPCSTR name, LPCSTR password, std::string& regverifyStr)
{
	int i_ret = 0;
	std::string strUrl;
	char *urlname = NULL, *urlpass = NULL;
	lo_UrlEncodeA(&urlname, name, strlen(name));
	lo_UrlEncodeA(&urlpass, password, strlen(password));
	base::SStringPrintf(&strUrl, "imap://%s:%s@imap.21cn.com",
		urlname ? urlname : "", urlpass ? urlpass : "");

	free(urlname);
	free(urlpass);

	vmime::ref<vmime::net::store> st;
	if (mailConnect(strUrl.c_str(), st)) {
	/*
		// Display some information about the connection
		vmime::ref <vmime::net::connectionInfos> ci = st->getConnectionInfos();

		std::cout << std::endl;
		std::cout << "Connected to '" << ci->getHost() << "' (port " << ci->getPort() << ")" << std::endl;
		std::cout << "Connection is " << (st->isSecuredConnection() ? "" : "NOT ") << "secured." << std::endl;
	*/
		// Open the default folder in this store
		vmime::ref <vmime::net::folder> f = st->getDefaultFolder();
	//		vmime::ref <vmime::net::folder> f = st->getFolder(vmime::utility::path("a"));
		
		int count = 0;
		try {
			f->open(vmime::net::folder::MODE_READ_ONLY);	//MODE_READ_WRITE
			count = f->getMessageCount();

			if (count > 5) {
				count = 5;
			}
		} catch (vmime::exception) {
			return i_ret;
		}

		for (int i = 1; i <= count; i++) {
			vmime::ref <vmime::net::message> msg = f->getMessage(i);
			f->fetchMessage(msg, vmime::net::folder::FETCH_FLAGS);

			if (msg->getFlags() == 0) {
				//unread
				f->fetchMessage(msg, vmime::net::folder::FETCH_ENVELOPE);

				std::string mailfrom;
				try {
					mailfrom = msg->getHeader()->From()->generate();
				} catch (vmime::exception) { /* In case the header field does not exist. */ }

				if (mailfrom.find("<passport@baidu.com>") != std::string::npos) {
					//from baidu
					bool matched = false;
					try {
						vmime::ref<vmime::message> pmsg = msg->getParsedMessage();
						vmime::messageParser mp(pmsg);
						for (int j = 0; j < mp.getTextPartCount(); j++) {
							vmime::ref<const vmime::textPart> tp = mp.getTextPartAt(j);
							//if (tp->getType().getSubType() == vmime::mediaTypes::TEXT_HTML) {
								//vmime::ref<const vmime::htmlTextPart> htp = tp.dynamicCast<const vmime::htmlTextPart>();
								
								vmime::string text;
								vmime::ref <const vmime::contentHandler> text_cth = tp->getText();
								
								vmime::utility::outputStreamStringAdapter text_os(text);
								vmime::utility::charsetFilteredOutputStream utf8filter_os(tp->getCharset(), vmime::charsets::UTF_8, text_os); //强制转换正文为utf8编码
								text_cth->extract(utf8filter_os);

								char *vurl = (char *)strstr(text.c_str(), "?regverify&vstr="); //inter a tag href have bug in 21cn "<a href=\"http://passport.baidu.com/v2/?regverify"
								if (vurl) {
									vurl += 16; // 9;
									char *vurlend = strstr(vurl, "&");	//'\"'
									if (vurlend) {
										vurlend[0] = 0;
									}
									regverifyStr = vurl;
									if (vurlend) {
										vurlend[0] = '&';
									}
									i_ret = i;
									matched = true;
									break;
								}
							//} else {
								// tp->getText();
							//}
						}
						if (matched) {
							break;
						}
						/**/
					} catch (vmime::exception) { /* In case the header field does not exist. */ }
				}
			}
		}

		try {
			f->close(true);
		} catch (vmime::exception) {
			//Nothing to do
		}
	}
	return i_ret;
}

/*
function hasPkCookie()
	pk_cookieValue = randomNum()
	setCookie(pkCookieName, pk_cookieValue, expiredays);

function hasSessionCookie()
	session_cookieValue = Date.parse(new Date()) + "-" + randomNum()
    setCookie(sessionCookieName, session_cookieValue, sessionExpiredays);

function randomNum() {
    return Math.round(Math.random() * 2147483647)
*/

void CMail21cn::SetWACookie()
{
	int randnum1 = base::RandInt(0, 2147483647);
	int randnum2 = base::RandInt(0, 2147483647);

	uint64 timestamp = time(NULL) * 1000; //Date.parse(new Date()) 就是 * 1000

	std::string session_cookieValue;
	base::SStringPrintf(&session_cookieValue, "%I64u-%u", timestamp, randnum2);

	m_taCMgr.add("_wa_pk_cookie", base::IntToString(randnum1).c_str());
	m_taCMgr.add("_wa_session_cookie", session_cookieValue.c_str());

	m_taCMgr.build();
}

int CMail21cn::Register(LPCSTR name, LPCSTR password, char** ret_info)
{
	CBuffer rqContent, rqHeader;

	//to get cookie and vcode
	if (CurlRequestGet("http://fmail.21cn.com/freeinterface/jsp/register/reg_moblie.htm", rqContent, &rqHeader) != TA_NET_OK) {
		return 1;
	}

	m_taCMgr.explain((const char *)rqHeader.GetBuffer());

	//get vcode
	rqContent.WriteZeroByte();
	const char *rqhtml = (const char *)rqContent.GetBuffer();

	const char *captchaToken = strstr(rqhtml, "<input name=\"captchaToken\"");
	if (captchaToken) {
		captchaToken += 26;
		captchaToken = strstr(captchaToken, "value=\"");
	}
	if (!captchaToken) {
		return 2;
	}

	captchaToken += 7;

	char *captchaTokenEnd = (char *)strstr(captchaToken, "\"");
	captchaTokenEnd[0] = 0;

	//to get cookie
	static const char vcodeurlf[] = "http://fmail.21cn.com/freeinterface/ValidateCodeServlet?token=%s";

	//DWORD t = time(NULL);
	std::string vcodeurl, ctoken;
	base::SStringPrintf(&vcodeurl, vcodeurlf, captchaToken);
	ctoken = captchaToken;

	bool bfirsttimeget = true;
	TA_REG_QUEUE_INFO rqi;

	do {
		rqContent.ClearBuffer();
		rqHeader.ClearBuffer();
		if (CurlRequestGet(vcodeurl.c_str(), rqContent, &rqHeader) != TA_NET_OK) {
			return 3;
		}
		if (bfirsttimeget) {
			bfirsttimeget = false;
			m_taCMgr.explain((const char *)rqHeader.GetBuffer());
			SetWACookie();
		}

		memset(&rqi, 0, sizeof(TA_REG_QUEUE_INFO));
		rqi.bVcodeData = rqContent.GetBuffer();
		rqi.nLen = rqContent.GetBufferLen();
		rqi.dwVcodeType = _VCODE_TYPE_REG;
		rqi.unknowvodelen = true;

		if (!VerifyRecognize(&rqi)) {
			return 40;
		}

		static const char vcodecheckurlf[] = "http://fmail.21cn.com/freeinterface/jsp/register/checkVerifyCodeforAjax.jsp";
		std::string vcodecheckdata;
		base::SStringPrintf(&vcodecheckdata, "verifyCode=%s", rqi.szVcode);

		rqContent.ClearBuffer();
		rqHeader.ClearBuffer();
		if (CurlRequestPost(vcodecheckurlf, vcodecheckdata.c_str(), rqContent, &rqHeader) != TA_NET_OK) {
			return 4;
		}
		
		rqContent.WriteZeroByte();
		//if(data !="0"){  验证码错误，请重新输入。
	} while (strstr((const char *)rqContent.GetBuffer(), "0") == NULL);

	static const char regdataf[] = "refererUrl=&returnUrl=%%2Findex.jsp&friendemail=null&userName=%s&passwd=%s&confirmPasswd=%s&verifyCode=%s&captchaToken=%s&agree2=y";
	static const char regurlf[] = "http://fmail.21cn.com/freeinterface/jsp/register/registerFormCommon.jsp";

	std::string regdata;
	//base::SStringPrintf(&regurl, regurlf, *);
	base::SStringPrintf(&regdata, regdataf, name, password, password, rqi.szVcode, ctoken.c_str());

	rqContent.ClearBuffer();
	rqHeader.ClearBuffer();
	if (CurlRequestPost(regurlf, regdata.c_str(), rqContent, &rqHeader) != TA_NET_OK) {
		return 5;
	}

	rqContent.WriteZeroByte();

	const char *retdata = (const char *)rqContent.GetBuffer();

	static const char alertflag[] = "<script>alert(\'";
	const char *szalert = strstr(retdata, alertflag);
	if (szalert) {
		szalert += 15;

		char *szalertend = (char *)strstr(szalert, "\'");
		if (szalertend) {
			szalertend[0] = 0;

			wchar_t *walert = NULL;
			lo_Utf82W(&walert, szalert);

			if (walert) {

				bool succeed = (wcsstr(walert, L"注册成功") != NULL);

				free(walert);

				if (succeed) {
					return 0;
				} else if (ret_info) {
					*ret_info = _strdup(szalert);
				}

				return 6;
			}
		}
	}

	return 7;
}