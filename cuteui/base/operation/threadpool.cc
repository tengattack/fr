
#include "threadpool.h"

#ifndef __drv_aliasesMem
#define __drv_aliasesMem
#endif

FORCEINLINE
VOID
InitializeListHead(
    __out PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

__checkReturn
BOOLEAN
FORCEINLINE
IsListEmpty(
    __in const LIST_ENTRY * ListHead
    )
{
    return (BOOLEAN)(ListHead->Flink == ListHead);
}

FORCEINLINE
BOOLEAN
RemoveEntryList(
    __in PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

FORCEINLINE
PLIST_ENTRY
RemoveHeadList(
    __inout PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



FORCEINLINE
PLIST_ENTRY
RemoveTailList(
    __inout PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


FORCEINLINE
VOID
InsertTailList(
    __inout PLIST_ENTRY ListHead,
    __inout __drv_aliasesMem PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


FORCEINLINE
VOID
InsertHeadList(
    __inout PLIST_ENTRY ListHead,
    __inout __drv_aliasesMem PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}


namespace operation{


CThreadPool::CThreadPool()
	: m_begintime(0)
	, m_itemcount(0)
	, m_complete_event(NULL)
	, m_destoried(true)
{
	memset(&m_threadpool, 0, sizeof(THREAD_POOL));
}

CThreadPool::~CThreadPool()
{
	
}

DWORD WINAPI CThreadPool::WorkerThread(PVOID pParam)
{
    PTHREAD_POOL pThreadPool = (PTHREAD_POOL)pParam;
    HANDLE Events[2];
      
    Events[0] = pThreadPool->QuitEvent;
    Events[1] = pThreadPool->WorkItemSemaphore;
  
    for(;;)
    {
        DWORD dwRet = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
  
        if(dwRet == WAIT_OBJECT_0)
            break;
  
        //
        // execute user's proc.
        //
  
        else if(dwRet == WAIT_OBJECT_0 +1)
        {
            PWORK_ITEM pWorkItem;
            PLIST_ENTRY pList;
  
            EnterCriticalSection(&pThreadPool->WorkItemLock);
            //_ASSERT(!IsListEmpty(&pThreadPool->WorkItemHeader));
            pList = RemoveHeadList(&pThreadPool->WorkItemHeader);
            LeaveCriticalSection(&pThreadPool->WorkItemLock);
  
            pWorkItem = CONTAINING_RECORD(pList, WORK_ITEM, List);
            pWorkItem->UserProc(pWorkItem->UserParam);
  
            InterlockedDecrement(&pThreadPool->WorkItemCount);
            free(pWorkItem);
        }
  
        else
        {
            //_ASSERT(0);
            break;
        }
    }
  
    return 0;
}

bool CThreadPool::Initialize(int threads)
{
	if (threads <= 0)
		return false;

	m_threadpool.QuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_threadpool.WorkItemSemaphore = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	m_threadpool.WorkItemCount = 0;
	InitializeListHead(&m_threadpool.WorkItemHeader);
	InitializeCriticalSection(&m_threadpool.WorkItemLock);
	m_threadpool.ThreadNum = threads;
	m_threadpool.ThreadsArray = (HANDLE*)malloc(sizeof(HANDLE) * threads);
	if (!m_threadpool.ThreadsArray) return false;
	memset(m_threadpool.ThreadsArray, 0, sizeof(HANDLE) * threads);

	for(int i = 0; i < threads; i++)
	{
		m_threadpool.ThreadsArray[i] = CreateThread(NULL, 0, WorkerThread, &m_threadpool, 0, NULL);
	}
  
	m_itemcount = 0;
	m_begintime = ::GetTickCount();
	m_destoried = false;

	return true;
}
void CThreadPool::Destroy()
{
	if (m_destoried)
		return;

	m_destoried = true;
    SetEvent(m_threadpool.QuitEvent);
  
    for(int i=0; i<m_threadpool.ThreadNum; i++)
    {
        WaitForSingleObject(m_threadpool.ThreadsArray[i], INFINITE);
        CloseHandle(m_threadpool.ThreadsArray[i]);
    }
  
    free(m_threadpool.ThreadsArray);
  
    CloseHandle(m_threadpool.QuitEvent);
    CloseHandle(m_threadpool.WorkItemSemaphore);
    DeleteCriticalSection(&m_threadpool.WorkItemLock);
  
    while(!IsListEmpty(&m_threadpool.WorkItemHeader))
    {
        PWORK_ITEM pWorkItem;
        PLIST_ENTRY pList;
          
        pList = RemoveHeadList(&m_threadpool.WorkItemHeader);
        pWorkItem = CONTAINING_RECORD(pList, WORK_ITEM, List);
          
        free(pWorkItem);
    }
}

bool CThreadPool::PostWorkItem(WORK_ITEM_PROC UserProc, PVOID UserParam)
{
	PWORK_ITEM pWorkItem = (PWORK_ITEM)malloc(sizeof(WORK_ITEM));
	if(pWorkItem == NULL)
		return false;
  
	pWorkItem->UserProc = UserProc;
	pWorkItem->UserParam = UserParam;
  
	EnterCriticalSection(&m_threadpool.WorkItemLock);
	InsertTailList(&m_threadpool.WorkItemHeader, &pWorkItem->List);
	LeaveCriticalSection(&m_threadpool.WorkItemLock);
  
	InterlockedIncrement(&m_threadpool.WorkItemCount);
	ReleaseSemaphore(m_threadpool.WorkItemSemaphore, 1, NULL);
  
	m_itemcount++;

	return true;
}

unsigned long CThreadPool::GetUseTime()
{
	return ::GetTickCount() - m_begintime;
}


};