
#include "stdafx.h"
#include "ThreadPool.h"

DWORD BeginTime;
LONG  ItemCount;
HANDLE CompleteEvent = NULL;

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


  
/************************************************************************/
  /* Test Our own thread pool.                                            */
  /************************************************************************/
  

  
  DWORD WINAPI WorkerThread(PVOID pParam)
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
  
  BOOL InitializeThreadPool(PTHREAD_POOL pThreadPool, LONG ThreadNum)
  {
      pThreadPool->QuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
      pThreadPool->WorkItemSemaphore = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
      pThreadPool->WorkItemCount = 0;
      InitializeListHead(&pThreadPool->WorkItemHeader);
      InitializeCriticalSection(&pThreadPool->WorkItemLock);
      pThreadPool->ThreadNum = ThreadNum;
      pThreadPool->ThreadsArray = (HANDLE*)malloc(sizeof(HANDLE) * ThreadNum);
  
      for(int i=0; i<ThreadNum; i++)
      {
          pThreadPool->ThreadsArray[i] = CreateThread(NULL, 0, WorkerThread, pThreadPool, 0, NULL);
      }
  
      return TRUE;
  }
  
  VOID DestroyThreadPool(PTHREAD_POOL pThreadPool)
  {
      SetEvent(pThreadPool->QuitEvent);
  
      for(int i=0; i<pThreadPool->ThreadNum; i++)
      {
          WaitForSingleObject(pThreadPool->ThreadsArray[i], INFINITE);
          CloseHandle(pThreadPool->ThreadsArray[i]);
      }
  
      free(pThreadPool->ThreadsArray);
  
      CloseHandle(pThreadPool->QuitEvent);
      CloseHandle(pThreadPool->WorkItemSemaphore);
      DeleteCriticalSection(&pThreadPool->WorkItemLock);
  
      while(!IsListEmpty(&pThreadPool->WorkItemHeader))
      {
          PWORK_ITEM pWorkItem;
          PLIST_ENTRY pList;
          
          pList = RemoveHeadList(&pThreadPool->WorkItemHeader);
          pWorkItem = CONTAINING_RECORD(pList, WORK_ITEM, List);
          
          free(pWorkItem);
      }
  }
  
  BOOL PostWorkItem(PTHREAD_POOL pThreadPool, WORK_ITEM_PROC UserProc, PVOID UserParam)
  {
      PWORK_ITEM pWorkItem = (PWORK_ITEM)malloc(sizeof(WORK_ITEM));
      if(pWorkItem == NULL)
          return FALSE;
  
      pWorkItem->UserProc = UserProc;
      pWorkItem->UserParam = UserParam;
  
      EnterCriticalSection(&pThreadPool->WorkItemLock);
      InsertTailList(&pThreadPool->WorkItemHeader, &pWorkItem->List);
      LeaveCriticalSection(&pThreadPool->WorkItemLock);
  
      InterlockedIncrement(&pThreadPool->WorkItemCount);
       ReleaseSemaphore(pThreadPool->WorkItemSemaphore, 1, NULL);
  
      return TRUE;
  }
