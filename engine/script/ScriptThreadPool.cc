
#include "stdafx.h"
#include "ScriptThreadPool.h"

CScriptThreadPool::CScriptThreadPool()
	: m_start_(false)
	, m_threads_(0)
{
}

CScriptThreadPool::~CScriptThreadPool()
{
}

bool CScriptThreadPool::Init(unsigned long threads)
{
	if (!m_start_ && threads > 0)
	{
		if (m_pool_.Initialize(threads))
		{
			m_start_ = true;
			m_threads_ = threads;
			return true;
		}
	}

	return false;
}

void CScriptThreadPool::Destory()
{
	if (m_start_)
	{
		m_start_ = false;
		m_pool_.Destroy();
	}
}

bool CScriptThreadPool::Add(CScriptBase *pScriptBase)
{
	if (m_start_)
	{
		if (m_pool_.PostWorkItem(ScriptSyncProc, (PVOID)pScriptBase))
		{
			return true;
		}
	}
	return false;
}

VOID CScriptThreadPool::ScriptSyncProc(PVOID Param)
{
	if (Param)
	{
		CScriptBase *pScriptBase = (CScriptBase *)Param;
		pScriptBase->SyncWork();
	}
}