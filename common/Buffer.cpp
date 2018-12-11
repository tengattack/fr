// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include <math.h>
#include "Buffer.h"

#include <assert.h>

#include "tcmalloc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
// #define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	CBuffer
// 
// DESCRIPTION:	Constructs the buffer with a default size
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
CBuffer::CBuffer()
	: m_free_ondestroy(true)
	, m_alloc_error(false)
	, m_nSize(0)
	, m_pPtr(NULL)
	, m_pBase(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	~CBuffer
// 
// DESCRIPTION:	Deallocates the buffer
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
CBuffer::~CBuffer()
{
	if (m_pBase && m_free_ondestroy) tc_free(m_pBase);
}
	

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Write
// 
// DESCRIPTION:	Writes data into the buffer
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::Write(const unsigned char* pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());

	if (m_alloc_error)
	{
		return FALSE;
	}

	CopyMemory(m_pPtr,pData,nSize);

	// Advance Pointer
	m_pPtr+=nSize;

	return nSize;
}

UINT CBuffer::WriteZeroByte()
{
	BYTE b = 0;
	return Write(&b, 1);
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Insert
// 
// DESCRIPTION:	Insert data into the buffer 
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
BOOL CBuffer::Insert(PBYTE pData, UINT nSize)
{
	ReAllocateBuffer(nSize + GetBufferLen());
	
	if (m_alloc_error)
	{
		return FALSE;
	}

	MoveMemory(m_pBase+nSize,m_pBase,GetMemSize() - nSize);
	CopyMemory(m_pBase,pData,nSize);

	// Advance Pointer
	m_pPtr+=nSize;

	return nSize;
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Read
// 
// DESCRIPTION:	Reads data from the buffer and deletes what it reads
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::Read(PBYTE pData, UINT nSize)
{
	// Trying to byte off more than ya can chew - eh?
	if (nSize > GetMemSize())
		return 0;

	// all that we have 
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();

		
	if (nSize)
	{
		// Copy over required amount and its not up to us
		// to terminate the buffer - got that!!!
		CopyMemory(pData,m_pBase,nSize);
		
		// Slide the buffer back - like sinking the data
		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);

		m_pPtr -= nSize;
	}
		
	DeAllocateBuffer(GetBufferLen());
	if (m_alloc_error)
	{
		return 0;
	}
	return nSize;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	GetMemSize
// 
// DESCRIPTION:	Returns the phyical memory allocated to the buffer
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::GetMemSize() 
{
	return m_nSize;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	GetBufferLen
// 
// DESCRIPTION:	Get the buffer 'data' length
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::GetBufferLen() const {
  if (m_pBase == nullptr) return 0;

  int nSize = m_pPtr - m_pBase;
  assert(nSize < 0);
  return nSize;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	ReAllocateBuffer
// 
// DESCRIPTION:	ReAllocateBuffer the Buffer to the requested size
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// TengAttack       140811		1.2			tc_malloc
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::ReAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize <= GetMemSize())
		return 0;

	// Allocate new size
	UINT nNewSize = (UINT)ceil(nRequestedSize / 1024.0) * 1024;
	UINT nBufferLen = GetBufferLen();

	// New Copy Data Over
	/*PBYTE pNewBuffer = (PBYTE) VirtualAlloc(NULL,nNewSize,MEM_COMMIT,PAGE_READWRITE);

	CopyMemory(pNewBuffer,m_pBase,nBufferLen);

	if (m_pBase)
		VirtualFree(m_pBase,0,MEM_RELEASE);

	// Hand over the pointer
	m_pBase = pNewBuffer;*/

	if (m_pBase)
	{
		unsigned char* pointer = m_pBase;
		m_pBase = reinterpret_cast<PBYTE>(tc_realloc(m_pBase, nNewSize));
		if (m_pBase == NULL)
		{
			free(pointer);
		}
	} else {
		m_pBase = reinterpret_cast<PBYTE>(tc_malloc(nNewSize));
	}

	if (m_pBase == NULL)
	{
		m_alloc_error = true;

		m_pPtr = NULL;
		m_nSize = 0;
	} else {
		// Realign position pointer
		m_pPtr = m_pBase + nBufferLen;
		m_nSize = nNewSize;
	}

	return m_nSize;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	DeAllocateBuffer
// 
// DESCRIPTION:	DeAllocates the Buffer to the requested size
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// TengAttack       140811		1.2			tc_malloc
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::DeAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetBufferLen())
		return 0;

	// Allocate new size +1024
	UINT nNewSize = (UINT)ceil(nRequestedSize / 1024.0) * 1024;

	if (nNewSize < GetMemSize())
		return 0;

	UINT nBufferLen = GetBufferLen();

	// New Copy Data Over
	/*PBYTE pNewBuffer = (PBYTE)VirtualAlloc(NULL,nNewSize,MEM_COMMIT,PAGE_READWRITE);

	
	CopyMemory(pNewBuffer,m_pBase,nBufferLen);

	VirtualFree(m_pBase,0,MEM_RELEASE);

	// Hand over the pointer
	m_pBase = pNewBuffer;*/
	
	if (m_pBase)
	{
		unsigned char* pointer = m_pBase;
		m_pBase = reinterpret_cast<PBYTE>(tc_realloc(m_pBase, nNewSize));
		if (m_pBase == NULL)
		{
			free(pointer);
		}
	} else {
		m_pBase = reinterpret_cast<PBYTE>(tc_malloc(nNewSize));
	}

	if (m_pBase == NULL)
	{
		m_alloc_error = true;
		m_pPtr = NULL;
		m_nSize = 0;
	} else {
		// Realign position pointer

		m_pPtr = m_pBase + nBufferLen;
		m_nSize = nNewSize;
	}

	return m_nSize;
}

void CBuffer::SetBufferLink(PBYTE pData, UINT buffer_size, UINT memory_size)
{
	if (m_pBase)
	{
		tc_free(m_pBase);
	}

	if (buffer_size == 0)
	{
		m_nSize = 0;
		if (pData) {
			m_pBase = pData;
		} else {
			m_pBase = NULL;
		}
		m_pPtr = m_pBase;
		return;
	}
	if (memory_size == 0)
	{
		m_nSize = tc_malloc_size(pData);
		if (m_nSize == (UINT)-1 || m_nSize == 0)
		{
			m_nSize = buffer_size;
		}
	} else {
		m_nSize = memory_size;
	}

	m_pBase = pData;
	m_pPtr = m_pBase + buffer_size;
}

void CBuffer::SetDestoryFree(bool enable)
{
	m_free_ondestroy = enable;
}

BOOL CBuffer::SetBufferLen(UINT newlen)
{
	if (newlen > m_nSize) {
		ReAllocateBuffer(newlen);

		if (m_alloc_error)
		{
			return FALSE;
		}
	}

	m_pPtr = m_pBase + newlen;

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Scan
// 
// DESCRIPTION:	Scans the buffer for a given byte sequence
// 
// RETURNS:		Logical offset
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
int CBuffer::Scan(PBYTE pScan,UINT nPos)
{
	if (nPos > GetBufferLen() )
		return -1;

	PBYTE pStr = (PBYTE) strstr((char*)(m_pBase+nPos),(char*)pScan);
	
	int nOffset = 0;

	if (pStr)
		nOffset = (pStr - m_pBase) + strlen((char*)pScan);

	return nOffset;
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	ClearBuffer
// 
// DESCRIPTION:	Clears/Resets the buffer
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CBuffer::ClearBuffer()
{
	// Force the buffer to be empty
	m_pPtr = m_pBase;

	DeAllocateBuffer(1024);
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Write
// 
// DESCRIPTION:	Writes a string a the end of the buffer
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
/*BOOL CBuffer::Write(CString& strData)
{
	int nSize = strData.GetLength();
	return Write((PBYTE) strData.GetBuffer(nSize), nSize);
}*/

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Insert
// 
// DESCRIPTION:	Insert a string at the beginning of the buffer
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
/*BOOL CBuffer::Insert(CString& strData)
{
	int nSize = strData.GetLength();
	return Insert((PBYTE) strData.GetBuffer(nSize), nSize);
}*/

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Copy
// 
// DESCRIPTION:	Copy from one buffer object to another...
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
void CBuffer::Copy(CBuffer& buffer)
{
	int nReSize = buffer.GetMemSize();
	int nSize = buffer.GetBufferLen();
	ClearBuffer();
	ReAllocateBuffer(nReSize);

	if (m_alloc_error)
	{
		return;
	}

	m_pPtr = m_pBase + nSize;

	CopyMemory(m_pBase,buffer.GetBuffer(),buffer.GetBufferLen());
}

////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	GetBuffer
// 
// DESCRIPTION:	Returns a pointer to the physical memory determined by the offset
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
PBYTE CBuffer::GetBuffer(UINT nPos)
{
	return m_pBase+nPos;
}


////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	GetBuffer
// 
// DESCRIPTION:	Returns a pointer to the physical memory determined by the offset
// 
// RETURNS:	
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
///////////////////////////////////////////////////////////////////////////////
bool CBuffer::FileWrite(LPCTSTR strFileName)
{
	bool writed = false;
	HANDLE hFile = CreateFile(strFileName,
				    GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL, 
					NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwWrites;
		writed = (bool)::WriteFile(hFile, m_pBase, GetBufferLen(), &dwWrites, NULL);
		CloseHandle(hFile);
	}
	/*if (!writed) {
		DWORD errorMessageID = ::GetLastError();
		LPTSTR messageBuffer = nullptr;
		size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&messageBuffer, 0, NULL);
		std::wstring message(messageBuffer, size);

		//Free the buffer.
		LocalFree(messageBuffer);

		MessageBox(NULL, message.c_str(), NULL, MB_ICONERROR);
	}*/
	return writed;
}



////////////////////////////////////////////////////////////////////////////////
// 
// FUNCTION:	Delete
// 
// DESCRIPTION:	Delete data from the buffer and deletes what it reads
// 
// RETURNS:		
// 
// NOTES:	
// 
// MODIFICATIONS:
// 
// Name				Date		Version		Comments
// N T ALMOND       270400		1.0			Origin
// 
////////////////////////////////////////////////////////////////////////////////
UINT CBuffer::Delete(UINT nSize)
{
	// Trying to byte off more than ya can chew - eh?
	if (nSize > GetMemSize())
		return 0;

	// all that we have 
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();

		
	if (nSize)
	{
		// Slide the buffer back - like sinking the data
		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);

		m_pPtr -= nSize;
	}
		
	DeAllocateBuffer(GetBufferLen());
	if (m_alloc_error)
	{
		return 0;
	}
	return nSize;
}
