
#include "stdafx.h"
#include "ScriptBase.h"
#include "ScriptThreadPool.h"

#include <v8.h>

using namespace v8;

extern CScriptThreadPool script_tpool;

CScriptBase::CScriptBase()
	: m_syncing_(false)
	, m_sync_result_(0)
{
}

CScriptBase::~CScriptBase()
{
}

int CScriptBase::SyncWork()
{
	//AutoLock autolock(m_lock);
	return 0;
}

void CScriptBase::SyncStart()
{
	m_lock.Acquire();
	m_sync_result_ = 0;
	m_syncing_ = true;
}

void CScriptBase::SyncFinish()
{
	m_lock.AssertAcquired();
	m_lock.Release();
	m_syncing_ = false;
}

bool CScriptBase::AddToSyncList()
{
	return script_tpool.Add(this);
}

void CScriptBase::AddBaseFunction(Handle<ObjectTemplate> obj)
{
	obj->SetAccessor(String::New("tag"), GetTag, SetTag);
}

Handle<Value> CScriptBase::GetTag(Local<String> property,
	const AccessorInfo &info)
{
	HandleScope handle_scope;
	Local<Object> self = info.Holder();
	CScriptBase* pBase = (CScriptBase *)self->GetPointerFromInternalField(0);
	if (!pBase) return v8::Undefined();

	return String::New((uint16_t *)pBase->GetTag());
}


void CScriptBase::SetTag(Local<String> property,
                               Local<Value> value,
                               const AccessorInfo &info)
{
	HandleScope handle_scope;
	Local<Object> self = info.Holder();
	CScriptBase* pBase = (CScriptBase *)self->GetPointerFromInternalField(0);
	if (!pBase) return;

	pBase->SetTag((wchar_t *)*(String::Value(value->ToString())));
}