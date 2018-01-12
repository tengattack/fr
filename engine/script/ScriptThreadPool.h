
#ifndef _TA_SCRIPT_THREAD_POOL_
#define _TA_SCRIPT_THREAD_POOL_

#include <base/operation/threadpool.h>
#include "ScriptBase.h"

class CScriptThreadPool {
public:
	CScriptThreadPool();
	virtual ~CScriptThreadPool();

	bool Init(unsigned long threads);
	void Destory();
	bool Add(CScriptBase *pScriptBase);

	static VOID ScriptSyncProc(PVOID Param);

protected:
	bool m_start_;
	unsigned long m_threads_;

	operation::CThreadPool m_pool_;
};

#endif	//_TA_SCRIPT_THREAD_POOL_