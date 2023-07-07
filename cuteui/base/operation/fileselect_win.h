
#ifndef _SNOW_CUTE_OPERATION_FILESELECT_H_
#define _SNOW_CUTE_OPERATION_FILESELECT_H_ 1

#include "../common.h"

namespace operation{
	enum OP_FS_MODE{
		kOpen = 0,
		kSave,
		kDir
	};
	class CFileSelect{
	public:
		CFileSelect(HWND hParent, OP_FS_MODE mode, LPCWSTR lpszFilter = NULL, LPCWSTR lpszTitle = NULL);
		~CFileSelect();

		void SetBufferSize(int nNewSize);

		bool Select();
		std::wstring& GetPath();

	protected:
		HWND m_hParent;
		OP_FS_MODE m_mode;
		std::wstring m_filter;
		std::wstring m_title;
		wchar_t *m_szBuffer;
		int m_buffer_size;

		std::wstring m_path;
	};
};

#endif