
#include "fileselect_win.h"
#include <Commdlg.h>
#include <Shlobj.h>

#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")

#define OP_FS_BUFFER_SIZE	1024
#define OP_FS_DEF_FILTER	L"所有文件 (*.*)|*.*||"

namespace operation{


CFileSelect::CFileSelect(HWND hParent, OP_FS_MODE mode, LPCWSTR lpszFilter, LPCWSTR lpszTitle)
	: m_hParent(hParent)
	, m_mode(mode)
	, m_filter(lpszFilter ? lpszFilter : OP_FS_DEF_FILTER)
	, m_title(lpszTitle ? lpszTitle : L"")
	, m_szBuffer(NULL)
	, m_buffer_size(OP_FS_BUFFER_SIZE)
{
	
}

CFileSelect::~CFileSelect()
{
	
}

void CFileSelect::SetBufferSize(int nNewSize)
{
	if (nNewSize > 0)
		m_buffer_size = nNewSize;
}

bool CFileSelect::Select()
{
	m_szBuffer = (wchar_t *)malloc(m_buffer_size * sizeof(wchar_t));
	if (!m_szBuffer) return false;
	m_szBuffer[0] = 0;
	bool bret = false;

	if (m_mode == kDir)
	{
		BROWSEINFOW bi;
		memset(&bi, 0, sizeof(BROWSEINFOW));
		bi.hwndOwner = m_hParent;
		bi.pszDisplayName = m_szBuffer;
		bi.lpszTitle = NULL;
		if (m_title.size() > 0)
		{
			bi.lpszTitle = m_title.c_str();
		}
		bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS | BIF_RETURNFSANCESTORS; 

		LPITEMIDLIST idl = SHBrowseForFolderW(&bi);

		if (idl != NULL)
		{
			SHGetPathFromIDListW(idl, m_szBuffer);
			if (lstrlenW(m_szBuffer) > 0)
			{
				bret = true;
			}
		}

	} else {

		wchar_t szFilter[1024];
		lstrcpynW(szFilter, m_filter.c_str(), 1024);
		int filter_len_ = lstrlenW(szFilter);
		for (int i = 0; i < filter_len_; i++)
		{
			if (szFilter[i] == '|')
			{
				szFilter[i] = 0;
			}
		}

		OPENFILENAMEW ofn;
		memset(&ofn, 0, sizeof(OPENFILENAMEW));
		ofn.lStructSize = sizeof(OPENFILENAMEW);
		ofn.hwndOwner = m_hParent;
		ofn.lpstrFile = m_szBuffer;
		ofn.nMaxFile = m_buffer_size;
		ofn.lpstrFilter = szFilter;
		ofn.nFilterIndex = 1;
		if (m_title.size() > 0)
		{
			ofn.lpstrTitle = m_title.c_str();
		}
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;


		switch (m_mode)
		{
		case kOpen:
			ofn.Flags |= OFN_HIDEREADONLY;
			bret = (bool)GetOpenFileNameW(&ofn);
			break;
		case kSave:
			ofn.Flags |= OFN_OVERWRITEPROMPT;
			bret = (bool)GetSaveFileNameW(&ofn);
			break;
		}
	}

	if (bret)
	{
		m_path = m_szBuffer;
	}

	free(m_szBuffer);
	return bret;
}

std::wstring& CFileSelect::GetPath()
{
	return m_path;
}

};