
#ifndef _SNOW_CUTE_CORE_USER_H_
#define _SNOW_CUTE_CORE_USER_H_ 1

#include "../base/common.h"

class CBuffer;

namespace core {

	enum UserListEncodingType {
		kULETUnset = 0,
		kULETGbk,
		kULETUtf8,
		kULETCount
	};

	//马甲文件的结构
	typedef struct _TAMJSTRUCT {
		char *szUserName;
		char *szCode;
		char *szCookie;
	} TAMJSTRUCT;

	typedef struct _TA_UTF8_MJSTRUCT {
		std::string username;
		std::string password;
		std::string cookie;
	} TA_UTF8_MJSTRUCT;

	class CUserList {
	public:
		CUserList(bool created = false, bool add_dup = true);	//是否为自创建模式, 添加时使用_strdup
		~CUserList();

		void Clear();
		TAMJSTRUCT& GetUser(unsigned long iIndex);
		TA_UTF8_MJSTRUCT GetUtf8User(unsigned long iIndex);

		inline TAMJSTRUCT* GetUserList() {
			return m_puserlist;
		}

		inline unsigned long GetCount() {
			return m_count;
		}
		
		inline UserListEncodingType GetEncodingType() {
			return m_encoding_type;
		}

		//这里设置后只对写入时有效
		inline void SetEncodingType(UserListEncodingType type) {
			m_encoding_type = type;
		}

		int ReadFromFile(LPCWSTR lpszPath);

		int ReadFromTextFile(LPCWSTR lpszPath);
		int ReadFromIdixFile(LPCWSTR lpszPath);
		bool WriteToIdixFile(LPCWSTR lpszPath);

		int ReadFromIaidFile(LPCWSTR lpszPath);
		int ReadFromIcidFile(LPCWSTR lpszPath);
		int ReadFromMSFile(LPCWSTR lpszPath);

		int ReadFromBuffer(BYTE* data, int len);

		static bool IsFileWithCookie(LPCWSTR lpszPath);

		int Add(const char* username, const char* password, const char* cookie); // created == true
		bool Clone(CUserList& userlist);	// userlist.m_created == true

	protected:

		bool m_created;
		bool m_add_dup;

		CBuffer* m_create_buf;
		unsigned long m_count;
		TAMJSTRUCT *m_puserlist;
		unsigned char *m_base_data;

		UserListEncodingType m_encoding_type;
	};

};

#endif