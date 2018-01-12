
#include "stdafx.h"
#include "163.h"

#include <string>
#include <time.h>
#include <net/net.h>

#include <common/Urlcode.h>

#include <base/string/stringprintf.h>
#include <base/string/string_number_conversions.h>
#include <base/rand_util.h>

#include <base/json/values.h>
#include <base/json/json_reader.h>

#include <vmime/vmime.hpp>

CMail163::CMail163()
{
}

CMail163::~CMail163()
{
}

const char* CMail163::GetUserAgent()
{
	return "Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.14 Safari/537.36";
}

//f.initEnv(R.envalue);
//this.initEnv=function(R){H="&env="+I.__$0(R)+"&t="};
//this.__$0=function(a){var q=10,w=a,n,m=w.length,f=new Date(),b=f.getTime(),c=b%q,h=(b-c)/q;if(c<1){c=1}c=b%q;var d=b%(q*q);h=(b-d)/q;h=h/q;d=(d-c)/q;var z=b+"",p=z.charAt(q),g=c+""+d+""+p,l=Number(g),e=l*Number(w),x=e+"",k="";for(n=(e+"").length-1;n>=0;n--){var o=x.charAt(n);k=k+o}var i=p+k+d+c,y=i.length,j=0,r="",v="";for(j=0;j<y;q++){r=r+i.charAt(j);j=j+2}for(j=1;j<y;j=j+2){v=v+i.charAt(j)}var u=r+v;u=i;var t=0,s="";for(t=0;t<u.length;t++){s=s+u.charAt(t)}return u}
std::string CMail163::GetEnvValue(const char* a)
{
	int q = 10;
	const char* w = a;
	int m = strlen(w);
	uint64 b = time(NULL) * 1000 + base::RandInt(0, 499);
	uint64 c = b % q;
	uint64 h = (b - c) / q;
	if (c < 1) {
		c = 1;
	}
	c = b % q;
	uint64 d = b % (q * q);
	h = (b - d) / q;
	h = h / q;
	d = (d - c) / q;

	std::string z = base::Int64ToString(b);
	char p = z[q];

	std::string g = base::Uint64ToString(c) + "" + base::Uint64ToString(d) + "" + p;	//其实就是将时间戳最后3位倒过来
	uint64 l = 0, numw = 0;
	base::StringToInt64(g, (int64 *)&l);
	base::StringToInt64(w, (int64 *)&numw);

	uint64 e = l * numw;
	std::string x = base::Uint64ToString(e);
	std::string k;
	for (int n = x.length() - 1; n >= 0; n--) {
		//var o=x.charAt(n);k=k+o
		k += x[n];
	}

	//var i=p+k+d+c,y=i.length,j=0,r="",v="";
	std::string i = p + k + base::Uint64ToString(d) + base::Uint64ToString(c);

	return i;

#if 0
	//无意义
	int y = i.length();
	int j = 0;
	std::string r, v;

	//for(j=0;j<y;q++){r=r+i.charAt(j);j=j+2}
	for (j = 0; j < y; q++) {
		r += i[j];
		j += 2;
	}

	//for(j=1;j<y;j=j+2){v=v+i.charAt(j)}
	for (j = 1; j < y; j += 2) {
		v += i[j];
	}

	//var u=r+v;u=i;var t=0,s="";for(t=0;t<u.length;t++){s=s+u.charAt(t)}return u}
	std::string u = r + v;
	u = i;

	return u;
#endif
}

int CMail163::GetRegverifyStr(LPCSTR name, LPCSTR password, std::string& regverifyStr)
{
	int i_ret = 0;
	std::string strUrl;
	char *urlname = NULL, *urlpass = NULL;
	lo_UrlEncodeA(&urlname, name, strlen(name));
	lo_UrlEncodeA(&urlpass, password, strlen(password));
	base::SStringPrintf(&strUrl, "imap://%s:%s@imap.163.com",
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

		f->open(vmime::net::folder::MODE_READ_ONLY);

		int count = f->getMessageCount();
		if (count > 5) {
			count = 5;
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
					try {
						std::string regmessage;

						vmime::ref <vmime::message> pmsg = msg->getParsedMessage();
						vmime::ref <vmime::body> body = pmsg->getBody();
						regmessage = body->generate();

						/*char *vurl = (char *)strstr(regmessage.c_str(), "<a href=\"http://passport.baidu.com/v2/?regverify");
						if (vurl) {
							vurl += 9;
							char *vurlend = strstr(vurl, "\"");
							if (vurlend) {
								vurlend[0] = 0;
							}
							regverifyUrl = vurl;
							i_ret = i;
							break;
						}*/
						char *vurl = (char *)strstr(regmessage.c_str(), "?regverify&vstr=");
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
							//matched = true;
							break;
						}
					} catch (vmime::exception) { /* In case the header field does not exist. */ }
				}
			}
		}

		f->close(true); 
	}
	return i_ret;
}

int CMail163::Register(LPCSTR name, LPCSTR password, char** ret_info)
{
	CBuffer rqContent, rqHeader;

/*
	if (CurlRequestGet("http://www.163.com/", rqContent, &rqHeader) != TA_NET_OK) {
		return 1;
	}
	m_taCMgr.explain((const char *)rqHeader.GetBuffer());

	rqContent.ClearBuffer();
	rqHeader.ClearBuffer();
*/

	//to get cookie and vcode
	if (CurlRequestGet("http://reg.email.163.com/unireg/call.do?cmd=register.entrance&from=163navi&regPage=163", rqContent, &rqHeader) != TA_NET_OK) {
		return 1;
	}

	m_taCMgr.explain((const char *)rqHeader.GetBuffer());
	const char *jsessionid = m_taCMgr.GetSession("JSESSIONID");
	if (!jsessionid) {
		return 2;
	}

	//get vcode
	rqContent.WriteZeroByte();
	const char *rqhtml = (const char *)rqContent.GetBuffer();

	const char *envalue = strstr(rqhtml, "envalue : \"");
	if (!envalue) {
		return 2;
	}

	envalue += 11;
	char *envalueend = (char *)strstr(envalue, "\"");
	envalueend[0] = 0;

	static const char vcodeurlf[] = "http://reg.email.163.com/unireg/call.do?cmd=register.verifyCode&v=common/verifycode/vc_en&env=%s&t=%u%03d";

	DWORD t = time(NULL);
	std::string vcodeurl;
	base::SStringPrintf(&vcodeurl, vcodeurlf, GetEnvValue(envalue).c_str(), t, base::RandInt(500, 999));

	rqContent.ClearBuffer();
	if (CurlRequestGet(vcodeurl.c_str(), rqContent) != TA_NET_OK) {
		return 3;
	}

	TA_REG_QUEUE_INFO rqi = {0};
	rqi.bVcodeData = rqContent.GetBuffer();
	rqi.nLen = rqContent.GetBufferLen();
	rqi.dwVcodeType = _VCODE_TYPE_REG;
	rqi.unknowvodelen = true;

	if (!VerifyRecognize(&rqi)) {
		return 40;
	}

	static const char regdataf[] = "name=%s&flow=main&uid=%s%%40163.com&password=%s&confirmPassword=%s&mobile=&vcode=%s&from=163navi";	//%%40 -> @
	static const char regurlf[] = "https://ssl.mail.163.com/regall/unireg/call.do;jsessionid=%s?cmd=register.start";

	std::string regurl, regdata;
	base::SStringPrintf(&regurl, regurlf, jsessionid);
	base::SStringPrintf(&regdata, regdataf, name, name, password, password, rqi.szVcode);

	rqContent.ClearBuffer();
	rqHeader.ClearBuffer();
	if (CurlRequestPost(regurl.c_str(), regdata.c_str(), rqContent, &rqHeader) != TA_NET_OK) {
		return 4;
	}

	rqContent.WriteZeroByte();
	const char *retdata = (const char *)rqContent.GetBuffer();
	if (retdata[0] == '{') {
		//json
		DictionaryValue *dv = (DictionaryValue *)base::JSONReader::Read(retdata, true);
		int icode = 5;
		if (dv) {
			if (dv->GetType() == Value::TYPE_DICTIONARY) {
				if (ret_info) {
					std::string strmsg;
					if (dv->GetString("msg", &strmsg)) {
						*ret_info = _strdup(strmsg.c_str());
					}
				}

				dv->GetInteger("code", &icode);
			}
		}
		return icode;
	} else {
		static const char icoflag[] = "<b class=\"icoShow";
		const char *szicocode = strstr(retdata, icoflag);
		if (szicocode) {
			szicocode += 17;
			char cicocode = szicocode[0];

			//只有 '1'~'5'
			if (cicocode == '5') {
				//成功！
				return 0;
			}

			if (ret_info) {
				const char *desc = strstr(retdata, "<p class=\"desc\">");
				if (desc) {
					desc += 16;
					char *descend = (char *)strstr(desc, "</p>");
					if (descend) {
						descend[0] = 0;
						*ret_info = _strdup(desc);
					}
				}
			}

			return (int)cicocode;
		}
	}

	return 6;
}