
#include "stdafx.h"
#include "mailbase.h"

#include <net/net.h>

#include <common/strconv.h>
#include <common/Urlcode.h>

#include <base/string/stringprintf.h>
#include <base/string/string_number_conversions.h>

#include <vmime/vmime.hpp>
#include <vmime/platforms/windows/windowsHandler.hpp>

static vmime::ref <vmime::net::session> g_session
	= vmime::create <vmime::net::session>();

#pragma comment(lib, "vmime.lib")
#pragma comment(lib, "libgnutls-28.lib")
#pragma comment(lib, "libgsasl.lib")

#if VMIME_HAVE_TLS_SUPPORT

// Certificate verifier (TLS/SSL)
class interactiveCertificateVerifier : public vmime::security::cert::defaultCertificateVerifier
{
public:

	void verify(vmime::ref <vmime::security::cert::certificateChain> chain)
	{
		try
		{
			setX509TrustedCerts(m_trustedCerts);

			defaultCertificateVerifier::verify(chain);
		}
		catch (vmime::exceptions::certificate_verification_exception&)
		{
			// Obtain subject's certificate
			vmime::ref <vmime::security::cert::certificate> cert = chain->getAt(0);
#if 0
			std::cout << std::endl;
			std::cout << "Server sent a '" << cert->getType() << "'" << " certificate." << std::endl;
			std::cout << "Do you want to accept this certificate? (Y/n) ";
			std::cout.flush();

			std::string answer;
			std::getline(std::cin, answer);

			if (answer.length() != 0 &&
			    (answer[0] == 'Y' || answer[0] == 'y'))
#endif
			{
				// Accept it, and remember user's choice for later
				if (cert->getType() == "X.509")
				{
					m_trustedCerts.push_back(cert.dynamicCast
						<vmime::security::cert::X509Certificate>());
				}

				return;
			}

			throw vmime::exceptions::certificate_verification_exception
				("User did not accept the certificate.");
		}
	}

private:

	static std::vector <vmime::ref <vmime::security::cert::X509Certificate> > m_trustedCerts;
};


std::vector <vmime::ref <vmime::security::cert::X509Certificate> >
	interactiveCertificateVerifier::m_trustedCerts;

#endif // VMIME_HAVE_TLS_SUPPORT


CMailBase::CMailBase()
{
	m_taCMgr.bind(this);
}

CMailBase::~CMailBase()
{
}

bool CMailBase::staticInit()
{
	vmime::platform::setHandler<vmime::platforms::windows::windowsHandler>();
	return true;
}

void CMailBase::staticUninit()
{
}

bool CMailBase::mailConnect(const char *protocolUrl, vmime::ref<vmime::net::store>& st)
{
	vmime::string urlString(protocolUrl);
	vmime::utility::url url(urlString);

	try
	{
		//vmime::ref <vmime::net::store> st;
		st = g_session->getStore(url);

#if VMIME_HAVE_TLS_SUPPORT

		// Enable TLS support if available
		st->setProperty("connection.tls", true);

		// Set the object responsible for verifying certificates, in the
		// case a secured connection is used (TLS/SSL)
		st->setCertificateVerifier
			(vmime::create <interactiveCertificateVerifier>());

#endif // VMIME_HAVE_TLS_SUPPORT

		// Connect to the mail store
		st->connect();
	}
	// VMime exception
	catch (vmime::exception& e)
	{
		//验证失败
		return false;
	}
	return true;
}

int CMailBase::getBaiduRegverifyStr(LPCSTR name, LPCSTR domain, LPCSTR password, std::string& regverifyStr)
{
	int i_ret = -1;
	std::string strUrl;
	char *urlname = NULL, *urlpass = NULL;
	lo_UrlEncodeA(&urlname, name, strlen(name));
	lo_UrlEncodeA(&urlpass, password, strlen(password));
	base::SStringPrintf(&strUrl, "imap://%s:%s@imap.%s",
		urlname ? urlname : "", urlpass ? urlpass : "", domain);

	free(urlname);
	free(urlpass);

	vmime::ref<vmime::net::store> st;
	if (mailConnect(strUrl.c_str(), st)) {
		i_ret = 0;
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
