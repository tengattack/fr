
#include "stdafx.h"
#include "users.h"
#include "base/file/file.h"
#include "base/file/filedata.h"
#include "base/string/stringprintf.h"

#include <filecommon/file_path.h>

#include <common/Buffer.h>
#include <common/strconv.h>
#include <lzmahelper.h>

//use ungzip
#include <zlib.h>
#include <net/net.h>

#include "ms_users.inl.cc"

namespace core {

static char *sz_encoding_flags[kULETCount] = {
	"", "encoding=gbk", "encoding=utf-8"
};

CUserList::CUserList(bool created, bool add_dup)
	: m_count(0)
	, m_created(created)
	, m_create_buf(NULL)
	, m_puserlist(NULL)
	, m_base_data(NULL)
	, m_encoding_type(kULETUnset)
	, m_add_dup(add_dup)
{
	if (m_created)
	{
		m_create_buf = new CBuffer;
	}
}

CUserList::~CUserList()
{
	Clear();
	if (m_created && m_create_buf)
	{
		delete m_create_buf;
	}
}

bool CUserList::IsFileWithCookie(LPCWSTR lpszPath)
{
	static wchar_t *sz_file_with_cookie_ext[] = {L"idix", L"iaid", L"icid", L"ms"};

	const wchar_t *filename = GetFileNameW(lpszPath);
	if (filename) {
		const wchar_t *fileext = GetFileNameExtW(filename);
		if (fileext) {
			for (int i = 0; i < (sizeof(sz_file_with_cookie_ext) / sizeof(wchar_t *)); i++) {
				if (_wcsicmp(fileext, sz_file_with_cookie_ext[i]) == 0) {
					return true;
				}
			}
		}
	}

	return false;
}

void CUserList::Clear()
{
	if (m_puserlist)
	{
		if (m_created)
		{
			if (m_add_dup) {
				for (int i = 0; i < m_count; i++)
				{
					if (m_puserlist[i].szUserName) free(m_puserlist[i].szUserName);
					if (m_puserlist[i].szCode) free(m_puserlist[i].szCode);
					if (m_puserlist[i].szCookie) free(m_puserlist[i].szCookie);
				}
			}
			m_create_buf->ClearBuffer();
		} else {
			free(m_puserlist);
		}
		m_puserlist = NULL;
	}
	if (m_base_data)
	{
		free(m_base_data);
		m_base_data = NULL;
	}
	SetEncodingType(kULETUnset);
	m_count = 0;
}

int CUserList::ReadFromTextFile(LPCWSTR lpszPath)
{
	if (m_created) return 0;

	base::CFile usersfile;
	base::CFileData fd;

	if (!usersfile.OpenW(base::kFileRead, lpszPath))
	{
		return 0;
	}
	int nTextLen = usersfile.GetFileSize();
	if (nTextLen <= 0) {
		return 0;
	}

	if (!fd.Read(usersfile)) {
		return 0;
	}
	
	std::string stext;

#ifdef NO_UTF8
	fd.ToText(stext);
	if (stext.empty()) return 0;

	SetEncodingType(kULETUnset);
#else
	fd.ToUtf8Text(stext);
	if (stext.empty()) return 0;

	SetEncodingType(kULETUtf8);
#endif

	usersfile.Close();

	return ReadFromBuffer((BYTE *)stext.c_str(), stext.length());
}

int CUserList::ReadFromBuffer(BYTE* data, int len)
{
	if (m_created) return 0;

	int i = 0, iCount = 0;
	int nTextLen = len;
	char* szData = (char *)malloc(nTextLen + 1);
	if (!szData) return 0;

	memcpy(szData, data, nTextLen);
	szData[nTextLen] = 0;
	m_base_data = (unsigned char *)szData;

	for (i = 0; i < nTextLen; i++)
	{
		if (szData[i] == '\n') iCount++;
	}
	if (szData[nTextLen - 1] != '\n') iCount++;

	m_puserlist = (TAMJSTRUCT *)malloc(sizeof(TAMJSTRUCT) * iCount);
	memset(m_puserlist, 0, iCount * sizeof(TAMJSTRUCT));

	char *szNextUserID = szData;
	char *szUserID, *szPassword;
	int iRealCount = 0;

	for (i = 0; i < iCount; i++)
	{
		if (!szNextUserID) break;
		szUserID = szNextUserID;
		szNextUserID = (char *)strstr((const char*)szUserID, "\r\n");
		if (szNextUserID) 
		{
			szNextUserID[0] = 0;
			szNextUserID += 2;
		}/*else{
			if (ndLen - (szNextUserID - szID) > 64)
				break;
		}*/
		szPassword = (char *)strstr((const char*)szUserID, ":");
		if (szPassword) 
		{
			szPassword[0] = 0;
			szPassword++;
		} else {
			//break;
			if (i == 0 && (GetEncodingType() == kULETUnset)) {
				//第一行设置编码
				for (int i = 0; i < kULETCount; i++) {
					if (_stricmp(szUserID, sz_encoding_flags[i]) == 0) {
						SetEncodingType((UserListEncodingType)i);
						break;
					}
				}
			}
			continue;
		}
		if (lstrlenA(szUserID) <= 0 || lstrlenA(szPassword) <= 0)
		{
			continue;
		}

		m_puserlist[iRealCount].szUserName = szUserID;
		m_puserlist[iRealCount].szCode = szPassword;
		iRealCount++;
		if (iRealCount >= iCount)
			break;
	}

	m_count = iRealCount;
	if (m_count == 0) {
		Clear();
	}
	return iRealCount;
}

int CUserList::ReadFromFile(LPCWSTR lpszPath)
{
	const wchar_t *filename = GetFileNameW(lpszPath);
	if (filename) {
		const wchar_t *fileext = GetFileNameExtW(filename);
		if (fileext) {
			if (_wcsicmp(fileext, L"idix") == 0) {
				return ReadFromIdixFile(lpszPath);
			} else if (_wcsicmp(fileext, L"iaid") == 0) {
				return ReadFromIaidFile(lpszPath);
			} else if (_wcsicmp(fileext, L"icid") == 0) {
				return ReadFromIcidFile(lpszPath);
			} else if (_wcsicmp(fileext, L"ms") == 0) {
				return ReadFromMSFile(lpszPath);
			}
		}
	}
	return ReadFromTextFile(lpszPath);
}

int CUserList::ReadFromIdixFile(LPCWSTR lpszPath)
{
	if (m_created) return 0;

	char *szIdixData = NULL;
	char *szData = NULL;

	base::CFile usersfile;
	if (!usersfile.OpenW(base::kFileRead, lpszPath))
	{
		return 0;
	}
	int nIdixLen = usersfile.GetFileSize();
	if (nIdixLen <= 0)
	{
		return 0;
	}

	int iCount = 0, i;
	szIdixData = (char *)malloc(nIdixLen);
	if (!szIdixData) return 0;
	if (!usersfile.Read((unsigned char *)szIdixData, nIdixLen))
	{
		free(szIdixData);
		return 0;
	}
	DWORD dwHeader[5];
	memcpy(dwHeader, szIdixData, 5 * sizeof(DWORD));
	if (dwHeader[0] != 0x58494449 || dwHeader[1] != (DWORD)nIdixLen || dwHeader[2] == 0)
	{
		free(szIdixData);
		return 0;
	}

	int status;
	size_t orilen = dwHeader[1] - (5 * sizeof(DWORD)) - dwHeader[4];
	size_t datalen = dwHeader[3];
	szData = (char *)malloc(datalen + 1);
	if (!szData)
	{
		free(szIdixData);
		return 0;
	}

	status = LzmaUncompress((BYTE *)szData, &datalen,
		(BYTE *)(szIdixData + 5 * sizeof(DWORD) + dwHeader[4]), &orilen,
		(BYTE *)(szIdixData + 5 * sizeof(DWORD)), dwHeader[4]);

	free(szIdixData);	//清空idix文件数据
	if (status != SZ_OK)
	{
		free(szData);
		return 0;
	}

	szData[datalen] = 0;
	m_base_data = (unsigned char *)szData;

	m_puserlist = (TAMJSTRUCT *)malloc(sizeof(TAMJSTRUCT) * dwHeader[2]);
	memset(m_puserlist, 0, dwHeader[2] * sizeof(TAMJSTRUCT));

	char *idStart = szData, *pwStart, *ckStart, *ckEnd;
	int iIn = 0;
	iCount = dwHeader[2];

	for (i = 0; i < iCount; i++)
	{
		pwStart = strstr(idStart, "\r\n");
		if (!pwStart) break;
		pwStart[0] = 0;
		pwStart += 2;

		ckStart = strstr(pwStart, "\r\n");
		if (!ckStart) break;
		ckStart[0] = 0;
		ckStart += 2;

		ckEnd = strstr(ckStart, "\r\n\r\n");
		if (!ckEnd) break;
		ckEnd[0] = 0;

		if (i == 0 && (GetEncodingType() == kULETUnset) && pwStart[0] == 0 && ckStart[0] == 0) {
			//第一行设置编码
			for (int j = 0; j < kULETCount; j++) {
				if (_stricmp(idStart, sz_encoding_flags[j]) == 0) {
					SetEncodingType((UserListEncodingType)j);
					break;
				}
			}
			if (GetEncodingType() != kULETUnset) {
				idStart = ckEnd + 4;
				//fix idix read less of 1 account
				i--;
				continue;
			}
		}

		m_puserlist[iIn].szUserName = idStart;
		m_puserlist[iIn].szCode = pwStart;
		if (ckStart[0] == 0)
		{
			//无Cookie时设为NULL
			m_puserlist[iIn].szCookie = NULL;
		} else {
			m_puserlist[iIn].szCookie = ckStart;
		}
		if (lstrlenA(idStart) <= 0)
		{
			continue;
		}

		idStart = ckEnd + 4;
		iIn++;
	}

	m_count = iIn;
	if (m_count == 0) {
		Clear();
	}
	return iIn;
}

bool CUserList::WriteToIdixFile(LPCWSTR lpszPath)
{
	if (!m_created) return false;
	if (GetCount() <= 0) return false;

	CBuffer buf;
	std::string strline;
	UserListEncodingType type = GetEncodingType();
	if (type != kULETUnset) {
		buf.Write((BYTE *)sz_encoding_flags[type], strlen(sz_encoding_flags[type]));
		buf.Write((BYTE *)"\r\n\r\n\r\n\r\n", 8);
	}

	for (unsigned long i = 0; i < GetCount(); i++) {
		TAMJSTRUCT& mj = GetUser(i);
		base::SStringPrintf(&strline, "%s\r\n%s\r\n%s\r\n\r\n", 
			(mj.szUserName ? mj.szUserName : ""),
			(mj.szCode ? mj.szCode : ""),
			(mj.szCookie ? mj.szCookie : ""));
		buf.Write((BYTE *)strline.c_str(), strline.length());
	}

	base::CFile file;
	if (file.OpenW(base::kFileCreate, lpszPath))
	{
		size_t propsize = LZMA_PROPS_SIZE;
		size_t csize = buf.GetBufferLen() * 3;
		BYTE props[LZMA_PROPS_SIZE];	//LZMA_PROPS_SIZE = 5
		BYTE *bcom = (BYTE *)malloc(csize);
		int status;
		/* *outPropsSize must be = 5 */
		status = LzmaCompress(bcom, &csize, buf.GetBuffer(), buf.GetBufferLen(), props, &propsize,
			5,		/* 0 <= level <= 9, default = 5 */
			65536,	/* default = (1 << 24) */
			3,		/* 0 <= lc <= 8, default = 3  */
			0,		/* 0 <= lp <= 4, default = 0  */
			2,		/* 0 <= pb <= 4, default = 2  */
			32,		/* 5 <= fb <= 273, default = 32 */
			1);		/* 1 or 2, default = 2 */
	
		if (status != SZ_OK) {
			free(bcom);
			file.Close();
			return false;
		}

		//IDIX 当前文件长度 马甲数 原始文件长度 属性大小
		DWORD dwHeader[5] = {0x58494449, 
			csize + (5 * sizeof(DWORD)) + propsize, //当前文件长度
			GetCount(), buf.GetBufferLen(), propsize};

		bool write_ = false;

		//头信息 属性表信息 数据
		write_ = file.Write((unsigned char *)dwHeader, 5 * sizeof(DWORD));

		if (write_) write_ = file.Write(props, propsize);
		if (write_) write_ = file.Write(bcom, csize);

		free(bcom);

		file.Close();
		return write_;
	} else {
		return false;
	}
}

int CUserList::Add(const char* username, const char* password, const char* cookie)
{
	if (!m_created) return 0;

	TAMJSTRUCT mj = {0};
	if (m_add_dup) {
		if (username) mj.szUserName = _strdup(username);
		if (password) mj.szCode = _strdup(password);
		if (cookie) mj.szCookie = _strdup(cookie);
	} else {
		mj.szUserName = (char *)username;
		mj.szCode = (char *)password;
		mj.szCookie = (char *)cookie;
	}

	if (m_create_buf->Write((unsigned char *)&mj, sizeof(TAMJSTRUCT)))
	{
		m_puserlist = (TAMJSTRUCT *)m_create_buf->GetBuffer();
		m_count++;
		return m_count - 1;
	} else {
		return -1;
	}
}

int CUserList::ReadFromIaidFile(LPCWSTR lpszPath)
{
	if (m_created) return 0;

	base::CFile usersfile;
	base::CFileData fd;

	if (!usersfile.OpenW(base::kFileRead, lpszPath))
	{
		return 0;
	}
	int nTextLen = usersfile.GetFileSize();
	if (nTextLen <= 0) {
		return 0;
	}

	if (!fd.Read(usersfile)) {
		return 0;
	}

	usersfile.Close();

	SetEncodingType(kULETUnset);

	CBuffer buf;
	if (ungzip((char *)fd.GetData(), fd.GetSize(), buf) == Z_OK) {

		buf.WriteZeroByte();
		buf.SetDestoryFree(false);

		int i = 0, iCount = 0;
		nTextLen = buf.GetBufferLen();
		char* szData = (char *)buf.GetBuffer();
		if (!szData) return 0;

		m_base_data = (unsigned char *)szData;

		for (i = 0; i < nTextLen; i++)
		{
			if (szData[i] == '\n') iCount++;
		}
		if (szData[nTextLen - 1] != '\n') iCount++;

		m_puserlist = (TAMJSTRUCT *)malloc(sizeof(TAMJSTRUCT) * iCount);
		memset(m_puserlist, 0, iCount * sizeof(TAMJSTRUCT));

		char *szNextUserID = szData;
		char *szUserID, *szPassword, *szCookie, *szTmp;
		int iRealCount = 0;

		while (true)
		{
			if (!szNextUserID) break;

			szUserID = szNextUserID;
			szTmp = szUserID;

			i = 0;
			while (szTmp[i] != NULL) {
				i++;
			}

			szPassword = szTmp + i + 1;
			szTmp = szPassword;

			i = 0;
			while (szTmp[i] != NULL) {
				i++;
			}

			szCookie = szTmp + i + 1;
			szNextUserID = strstr(szCookie, "\r\n");

			if (szNextUserID) {
				szNextUserID[0] = 0;
				szNextUserID += 2;
			}

			if (strlen(szUserID) <= 0) {
				continue;
			}

			if (szPassword && strlen(szPassword) <= 0) {
				szPassword = NULL;
			}

			m_puserlist[iRealCount].szUserName = szUserID;
			m_puserlist[iRealCount].szCode = szPassword;
			m_puserlist[iRealCount].szCookie = szCookie;
			iRealCount++;
			if (iRealCount >= iCount)
				break;
		}

		m_count = iRealCount;
		if (m_count == 0) {
			Clear();
		}
		return iRealCount;
	}

	return 0;
}

int CUserList::ReadFromIcidFile(LPCWSTR lpszPath)
{
	if (m_created) return 0;

	base::CFile usersfile;
	base::CFileData fd;

	if (!usersfile.OpenW(base::kFileRead, lpszPath))
	{
		return 0;
	}
	int nTextLen = usersfile.GetFileSize();
	if (nTextLen <= 0) {
		return 0;
	}

	if (!fd.Read(usersfile)) {
		return 0;
	}
	
	std::string stext;

#ifdef NO_UTF8
	fd.ToText(stext);
	if (stext.empty()) return 0;

	SetEncodingType(kULETUnset);
#else
	fd.ToUtf8Text(stext);
	if (stext.empty()) return 0;

	SetEncodingType(kULETUtf8);
#endif

	usersfile.Close();
	
	int i = 0, iCount = 0;
	nTextLen = stext.length();
	char* szData = (char *)malloc(nTextLen + 1);
	if (!szData) return 0;

	memcpy(szData, stext.c_str(), nTextLen);
	szData[nTextLen] = 0;
	m_base_data = (unsigned char *)szData;

	for (i = 0; i < nTextLen; i++)
	{
		if (szData[i] == '\n') iCount++;
	}
	if (szData[nTextLen - 1] != '\n') iCount++;

	m_puserlist = (TAMJSTRUCT *)malloc(sizeof(TAMJSTRUCT) * iCount);
	memset(m_puserlist, 0, iCount * sizeof(TAMJSTRUCT));

	char *szNextUserID = szData;
	char *szUserID, *szPassword, *szCookie, *szTmp;
	int iRealCount = 0;

	for (i = 0; i < iCount; i++)
	{
		if (!szNextUserID) break;

		szUserID = strchr(szNextUserID, '(');
		szNextUserID = strstr(szNextUserID, "\r\n");

		if (szUserID) {
			szUserID++;
		} else {
			continue;
		}
		if (szNextUserID) 
		{
			szNextUserID[0] = 0;
			szNextUserID += 2;
		}/*else{
			if (ndLen - (szNextUserID - szID) > 64)
				break;
		}*/
		szPassword = strchr(szUserID, ',');
		if (szPassword) {
			szPassword[0] = 0;
			szPassword++;

			szTmp = strchr(szPassword, ')');
			if (szTmp) {
				szTmp[0] = 0;
				szTmp++;

				if (strlen(szPassword) <= 0) {
					szPassword = NULL;
				}
			} else {
				continue;
			}
		} else {
			continue;
		}
		if (strlen(szUserID) <= 0) {
			continue;
		}

		szCookie = strchr(szTmp, '[');
		if (szCookie) {
			szCookie++;

			szTmp = strchr(szCookie, ']');
			if (szTmp) {
				szTmp[0] = 0;
			}
		} else {
			continue;
		}

		m_puserlist[iRealCount].szUserName = szUserID;
		m_puserlist[iRealCount].szCode = szPassword;
		m_puserlist[iRealCount].szCookie = szCookie;
		iRealCount++;
		if (iRealCount >= iCount)
			break;
	}

	m_count = iRealCount;
	if (m_count == 0) {
		Clear();
	}
	return iRealCount;
}

int CUserList::ReadFromMSFile(LPCWSTR lpszPath)
{
	if (m_created) return 0;

	base::CFile usersfile;
	base::CFileData fd;

	if (!usersfile.OpenW(base::kFileRead, lpszPath))
	{
		return 0;
	}
	int nTextLen = usersfile.GetFileSize();
	if (nTextLen <= 0) {
		return 0;
	}

	if (!fd.Read(usersfile)) {
		return 0;
	}
	usersfile.Close();

	if (ms_check_flags(fd.GetData(), fd.GetSize())) {
		CBuffer buf;
		if (ms_decompress(fd.GetData(), fd.GetSize(), buf)) {
			if (ms_decode(buf.GetBuffer(), buf.GetBufferLen())) {
				Clear();
				m_count = ms_text_parse(buf, &m_puserlist);
				if (m_count > 0) {
					buf.SetDestoryFree(false);
					m_base_data = buf.GetBuffer();
#ifdef NO_UTF8
					SetEncodingType(kULETUnset);
#else
					SetEncodingType(kULETUtf8);
#endif
					return m_count;
				}
			}
		}
	}
	return 0;
}

bool CUserList::Clone(CUserList& userlist)
{
	userlist.Clear();
	userlist.SetEncodingType(GetEncodingType());
	for (int i = 0; i < m_count; i++)
	{
		userlist.Add(
			m_puserlist[i].szUserName,
			m_puserlist[i].szCode,
			m_puserlist[i].szCookie);
	}
	return m_count == userlist.GetCount();
}

TAMJSTRUCT& CUserList::GetUser(unsigned long iIndex)
{
	return m_puserlist[iIndex];
}

TA_UTF8_MJSTRUCT CUserList::GetUtf8User(unsigned long iIndex)
{
	TA_UTF8_MJSTRUCT mj;
	if (GetEncodingType() == kULETUtf8) {
		mj.username = m_puserlist[iIndex].szUserName;
	} else {
		char *utf8name = NULL;
		lo_C2Utf8(&utf8name, m_puserlist[iIndex].szUserName);
		if (utf8name) {
			mj.username = utf8name;
			free(utf8name);
		}
	}

	//这2者一般没有区别（不排除Cookie中有UTF8字符，但不影响
	mj.password = m_puserlist[iIndex].szCode;
	mj.cookie = m_puserlist[iIndex].szCookie;

	return mj;
}

};