
// “构造”
using namespace v8;

Handle<Value> cTAHttpConstructor(const Arguments& args)
{
	HandleScope handle_scope;
	Handle<Object> obj = args.This();
	
	//testq *ptr = new testq;
	CTAV8Http *ptr = new CTAV8Http;
	//obj->SetInternalFieldCount(1);
	obj->SetInternalField(0, External::New(ptr));
	return obj;
}

// 类属性Getter函数
Handle<Value> GetTAHttpResponseText(Local<String> property,
	const AccessorInfo &info)
{
	HandleScope handle_scope;
	Handle<Object> self = info.Holder();
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	
	if (!pTaHttp) return v8::Undefined();

	if (pTaHttp->szResponseText)
		return v8func::MbToV8String(pTaHttp->szResponseText);
	else
		return String::New("");
}

Handle<Value> GetTAHttpResponseHeader(Local<String> property,
	const AccessorInfo &info)
{
	HandleScope handle_scope;
	Local<Object> self = info.Holder();
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);

	if (!pTaHttp) return v8::Undefined();

	if (pTaHttp->szResponseHeader)
		return v8func::MbToV8String(pTaHttp->szResponseHeader);
	else
		return String::New("");
}

// 类方法

Handle<Value> TAHttpOpen(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	/*Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	CTAV8Http* pTaHttp = (CTAV8Http*)wrap->Value();*/
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);

	if (!pTaHttp) return v8::Undefined();

	if (args.Length() >= 2)
	{
		Handle<String> s = args[1]->ToString();

		char *szUrl = NULL;
		lo_W2C(&szUrl, (const wchar_t *)*(String::Value(s)));

		bool bret = pTaHttp->Open(*(String::AsciiValue(args[0]->ToString())), szUrl);

		free(szUrl);

		return (bret ? v8::True() : v8::False());
	} else {
		return v8::Undefined();
	}
}

Handle<Value> TAHttpSetRequestHeader(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	/*Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	CTAV8Http* pTaHttp = (CTAV8Http*)wrap->Value();*/
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);

	if (!pTaHttp) return v8::Undefined();

	if (args.Length() >= 2)
	{
		Handle<String> s = args[1]->ToString();
		char *szData = NULL;
		lo_W2C(&szData, (const wchar_t *)*(String::Value(s)));

		pTaHttp->SetRequestHeader(*(String::AsciiValue(args[0]->ToString())), szData);

		free(szData);

		return v8::True();
	} else {
		return v8::False();
	}

	return Undefined();
}

Handle<Value> TAHttpSend(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	/*Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	CTAV8Http* pTaHttp = (CTAV8Http*)wrap->Value();*/
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	
	if (!pTaHttp) return v8::Undefined();

	bool b_ret = false;
	if (args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();
		char *szData = NULL;
		lo_W2C(&szData, (const wchar_t *)*(String::Value(s)));
		b_ret = pTaHttp->Send(szData);
		free(szData);
	} else {
		b_ret = pTaHttp->Send();
	}

	if (b_ret)
		return v8::True();
	else
		return v8::False();
}

Handle<Value> TAHttpSyncSend(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	/*Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	CTAV8Http* pTaHttp = (CTAV8Http*)wrap->Value();*/
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	if (!pTaHttp) return v8::Undefined();

	bool b_ret = false;
	if (args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();
		char *szData = NULL;
		lo_W2C(&szData, (const wchar_t *)*(String::Value(s)));
		b_ret = pTaHttp->SyncSend(szData);
		free(szData);
	} else {
		b_ret = pTaHttp->SyncSend();
	}

	if (b_ret)
		return v8::True();
	else
		return v8::False();
}

Handle<Value> TAHttpIsSyncing(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	/*Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	CTAV8Http* pTaHttp = (CTAV8Http*)wrap->Value();*/
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	if (!pTaHttp) return v8::Undefined();

	if (pTaHttp->IsSyncing())
		return v8::True();
	else
		return v8::False();
}

Handle<Value> TAHttpGetSyncResult(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	/*Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	CTAV8Http* pTaHttp = (CTAV8Http*)wrap->Value();*/
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	if (!pTaHttp) return v8::Undefined();

	return v8::Int32::New(pTaHttp->GetSyncResult());
}

Handle<Value> TAHttpClean(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	/*Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	CTAV8Http* pTaHttp = (CTAV8Http*)wrap->Value();*/
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	if (!pTaHttp) return v8::Undefined();

	pTaHttp->Clean();

	return Undefined();
}

Handle<Value> TAHttpRemoveProxy(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	if (!pTaHttp) return v8::Undefined();

	pTaHttp->RemoveProxy();

	return Undefined();
}

Handle<Value> TAHttpSetProxy(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	if (!pTaHttp) return v8::Undefined();

	char* ipaddress = NULL;
	DWORD port = 80;
	SnowProxyType type = kHttp;
	int c = args.Length();
	if (c == 1 && args[0]->IsObject())
	{
		//ProxyObj
		Local<Object> proxyobj = args[0]->ToObject();
		Local<Value> ip = proxyobj->Get(String::New("ipaddress"));

		char *szData = NULL;
		lo_W2C(&szData, (const wchar_t *)*(String::Value(ip)));

		if (lstrlenA(szData))
		{
			ipaddress = szData;
		} else {
			free(szData);
		}

		if (ipaddress)
		{
			Local<Value> pr = proxyobj->Get(String::New("port"));
			Local<Value> ty = proxyobj->Get(String::New("type"));

			port = pr->Int32Value();
			type = (SnowProxyType)ty->Int32Value();
		}
		
	} else if (c >= 1 && args[0]->IsString()){
		Handle<String> s = args[0]->ToString();
		char *szData = NULL;
		lo_W2C(&szData, (const wchar_t *)*(String::Value(s)));

		if (lstrlenA(szData))
		{
			ipaddress = szData;
		} else {
			free(szData);
		}

		if (ipaddress)
		{
			if (c >= 2)
			{
				port = args[1]->Int32Value();
			}
			if (c >= 3)
			{
				type = (SnowProxyType)args[2]->Int32Value();
			}
		}
	}

	if (ipaddress)
	{
		if (pTaHttp->SetProxy(ipaddress, port, type))
		{
			free(ipaddress);
			return v8::True();
		} else {
			free(ipaddress);
			return v8::False();
		}
	}
	
	return Undefined();
}

Handle<Value> TAHttpDispose(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	/*Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	CTAV8Http* pTaHttp = (CTAV8Http*)wrap->Value();*/
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);
	
	if (pTaHttp)
	{
		delete pTaHttp;
		self->SetInternalField(0, External::New(NULL));
	}

	return Undefined();
}

/*Handle<Value> GetTAHttpPointer(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CTAV8Http* pTaHttp = (CTAV8Http*)self->GetPointerFromInternalField(0);

	return Uint32::New((unsigned int)pTaHttp);
}*/


//加入自定义类

void AddTAHttp(Handle<ObjectTemplate> global)
{
	HandleScope handle_scope;
	Handle<FunctionTemplate> myc = FunctionTemplate::New(cTAHttpConstructor);
	
	myc->SetClassName(String::New("CTAHttp"));
	global->Set(String::New("CTAHttp"), myc);

	Handle<ObjectTemplate> proto = myc->PrototypeTemplate();

	proto->Set("Open", FunctionTemplate::New(TAHttpOpen));
	proto->Set("SetRequestHeader", FunctionTemplate::New(TAHttpSetRequestHeader));
	proto->Set("Send", FunctionTemplate::New(TAHttpSend));
	proto->Set("SyncSend", FunctionTemplate::New(TAHttpSyncSend));
	proto->Set("IsSyncing", FunctionTemplate::New(TAHttpIsSyncing));
	proto->Set("GetSyncResult", FunctionTemplate::New(TAHttpGetSyncResult));

	proto->Set("SetProxy", FunctionTemplate::New(TAHttpSetProxy));
	proto->Set("RemoveProxy", FunctionTemplate::New(TAHttpRemoveProxy));

	proto->Set("Clean", FunctionTemplate::New(TAHttpClean));
	proto->Set("dispose", FunctionTemplate::New(TAHttpDispose));
	//proto->Set("Pointer", FunctionTemplate::New(GetTAHttpPointer));

	Handle<ObjectTemplate> inst = myc->InstanceTemplate();

	inst->SetInternalFieldCount(1);

	inst->SetAccessor(String::New("ResponseText"), GetTAHttpResponseText);
	inst->SetAccessor(String::New("ResponseHeader"), GetTAHttpResponseHeader);
	
	CScriptBase::AddBaseFunction(inst);
}