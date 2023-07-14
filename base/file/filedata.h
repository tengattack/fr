
#ifndef _SNOW_CUTE_BASE_FILEDATA_H_
#define _SNOW_CUTE_BASE_FILEDATA_H_ 1

#include "../common.h"
#include "../basictypes.h"
#include "file.h"

namespace base{
	enum TEXT_FORMAT{
		kTFUnknow = 0,
		kTFASCII = 1,
		kTFUnicode = 2,
		kTFUtf8 = 3
	};
	class CFileData{
	public:
		CFileData();
		~CFileData();

		void Clear();
		bool Read(CFile& file);

		unsigned char* GetData();
		void ToText(std::wstring& text);
		void ToText(std::string& text);
		void ToUtf8Text(std::string& text);

		TEXT_FORMAT GetDataFormat();

		uint32 GetSize();

	protected:
		uint32 m_size;
		unsigned char* m_data;
	};
};

#endif