
#ifndef _SNOW_CUTE_OPERATION_THREADPOOL_H_
#define _SNOW_CUTE_OPERATION_THREADPOOL_H_ 1

#include "../common.h"

namespace operation{
	typedef struct _THREAD_POOL
	{
		HANDLE QuitEvent;
		HANDLE WorkItemSemaphore;
  
		LONG WorkItemCount;
		LIST_ENTRY WorkItemHeader;
		CRITICAL_SECTION WorkItemLock;
  
		LONG ThreadNum;
		HANDLE *ThreadsArray;
  
	}THREAD_POOL, *PTHREAD_POOL;
  
	typedef void (*WORK_ITEM_PROC)(PVOID Param);
  
	typedef struct _WORK_ITEM
	{
		LIST_ENTRY List;
  
		WORK_ITEM_PROC UserProc;
		PVOID UserParam;
      
	}WORK_ITEM, *PWORK_ITEM;

	class CThreadPool{
	public:
		CThreadPool();
		~CThreadPool();

		bool Initialize(int threads);
		void Destroy();
		bool PostWorkItem(WORK_ITEM_PROC UserProc, PVOID UserParam);

		unsigned long GetUseTime();

		static DWORD WINAPI WorkerThread(PVOID pParam);

	protected:
		bool m_destoried;
		unsigned long m_begintime;
		unsigned long m_itemcount;
		HANDLE m_complete_event;

		THREAD_POOL m_threadpool;
	};
};

#endif