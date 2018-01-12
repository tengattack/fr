
#include "stdafx.h"
#include "ScriptFileDialog.h"

#include <base/operation/fileselect.h>

CScriptFileDialog::CScriptFileDialog()
{
}

CScriptFileDialog::~CScriptFileDialog()
{
}

bool CScriptFileDialog::Select(bool IsOpen, LPCWSTR lpszFilter, LPCWSTR lpszTitle)
{
	operation::CFileSelect fs(NULL, 
		(IsOpen ? operation::kOpen : operation::kSave),
		lpszFilter, lpszTitle);

	bool bret = fs.Select();
	if (bret) {
		m_path_ = fs.GetPath();
	}
	return bret;
}