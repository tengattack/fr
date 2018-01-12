
#ifndef _TA_SCRIPT_BASE_
#define _TA_SCRIPT_BASE_

#include <string>
#include <v8.h>
#include <base/lock.h>

class CScriptBase {
public:
	CScriptBase();
	virtual ~CScriptBase();

	bool AddToSyncList();

	virtual int SyncWork();

	virtual void SyncStart();
	virtual void SyncFinish();

	inline void SetSyncResult(int result)
	{
		m_sync_result_ = result;
	}

	inline int GetSyncResult()
	{
		return m_sync_result_;
	}

	inline bool IsSyncing()
	{
		return m_syncing_;
	}

	inline LPCWSTR GetTag()
	{
		return m_tag_.c_str();
	}

	inline void SetTag(LPCWSTR lpszTag)
	{
		m_tag_ = lpszTag;
	}

	static void AddBaseFunction(v8::Handle<v8::ObjectTemplate> obj);
	static v8::Handle<v8::Value> GetTag(v8::Local<v8::String> property,
			const v8::AccessorInfo &info);
	static void SetTag(v8::Local<v8::String> property,
                               v8::Local<v8::Value> value,
                               const v8::AccessorInfo &info);
protected:
	Lock m_lock;
	bool m_syncing_;
	int m_sync_result_;

	std::wstring m_tag_;
};

#endif	//_TA_SCRIPT_BASE_