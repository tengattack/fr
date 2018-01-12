
#pragma once

//#include "stdafx.h"
#include <windows.h>
#include "process.h"
//#include "wdm.h"

extern DWORD BeginTime;
extern LONG  ItemCount;
extern HANDLE CompleteEvent;

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
  
typedef VOID (*WORK_ITEM_PROC)(PVOID Param);
  
typedef struct _WORK_ITEM
{
    LIST_ENTRY List;
  
    WORK_ITEM_PROC UserProc;
    PVOID UserParam;
      
}WORK_ITEM, *PWORK_ITEM;

DWORD WINAPI WorkerThread(PVOID pParam);
BOOL InitializeThreadPool(PTHREAD_POOL pThreadPool, LONG ThreadNum);
VOID DestroyThreadPool(PTHREAD_POOL pThreadPool);
BOOL PostWorkItem(PTHREAD_POOL pThreadPool, WORK_ITEM_PROC UserProc, PVOID UserParam);