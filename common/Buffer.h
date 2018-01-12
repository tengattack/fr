// Buffer.h: interface for the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUFFER_H__829F6693_AC4D_11D2_8C37_00600877E420__INCLUDED_)
#define AFX_BUFFER_H__829F6693_AC4D_11D2_8C37_00600877E420__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

class CBuffer  
{
// Attributes
protected:
	PBYTE	m_pBase;
	PBYTE	m_pPtr;
	UINT	m_nSize;

	bool m_free_ondestroy;
	bool m_alloc_error;

// Methods
protected:
	UINT ReAllocateBuffer(UINT nRequestedSize);
	UINT DeAllocateBuffer(UINT nRequestedSize);
	
public:
	void ClearBuffer();

	UINT Delete(UINT nSize);
	UINT Read(PBYTE pData, UINT nSize);
	BOOL Write(PBYTE pData, UINT nSize);
	BOOL WriteZeroByte();
//	BOOL Write(CString& strData);
	UINT GetBufferLen();
	UINT GetMemSize();
	int Scan(PBYTE pScan,UINT nPos);
	BOOL Insert(PBYTE pData, UINT nSize);
//	BOOL Insert(CString& strData);

	void Copy(CBuffer& buffer);	

	PBYTE GetBuffer(UINT nPos=0);
	void SetBufferLink(PBYTE pData, UINT buffer_size, UINT memory_size = 0);	//直接设置内容数据
	void SetDestoryFree(bool enable = true);
	BOOL SetBufferLen(UINT newlen);

	CBuffer();
	virtual ~CBuffer();

	bool FileWrite(LPCTSTR strFileName);

};

#endif // !defined(AFX_BUFFER_H__829F6693_AC4D_11D2_8C37_00600877E420__INCLUDED_)
