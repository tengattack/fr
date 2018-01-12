
#ifndef _TA_SNOW_SCRIPT_FILE_DIALOG_H_
#define _TA_SNOW_SCRIPT_FILE_DIALOG_H_
//-----------FileDialog for V8Engine--------------

Handle<Value> ClsFileDialogConstructor(const Arguments& args)
{
	HandleScope handle_scope;
	Handle<Object> obj = args.This();

	CScriptFileDialog *psfd = new CScriptFileDialog;
	obj->SetInternalField(0, External::New(psfd));
	return obj;
}

Handle<Value> ClsFileDialog_Select(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFileDialog* psfd = (CScriptFileDialog*)self->GetPointerFromInternalField(0);
	
	if (!psfd) return v8::Undefined();

	bool bret = false;
	int c = args.Length();
	if (c > 0)
	{
		if (c == 1)
		{
			bret = psfd->Select(args[0]->BooleanValue());
		} else if (c > 1) {
			//不知为什么需要转换
			wchar_t *title = NULL;
			wchar_t *filter = NULL;
			
			Handle<String> hfilter = args[1]->ToString();
			lo_V8S2W(&filter, hfilter);
			
			if (c > 2) {
				Handle<String> htitle = args[2]->ToString();
				//title = utf8ToWcString(*(String::Utf8Value(htitle)));
				lo_V8S2W(&title, htitle);
			}

			bret = psfd->Select(args[0]->BooleanValue(), filter, title);

			if (title) free(title);
			if (filter) free(filter);
		}
	} else {
		return v8::Undefined();
	}
	

	if (bret)
	{
		return v8::True();
	} else {
		return v8::False();
	}
}

Handle<Value> ClsFileDialog_GetPathName(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFileDialog* psfd = (CScriptFileDialog*)self->GetPointerFromInternalField(0);
	
	if (!psfd) return v8::Undefined();

	return String::New((const uint16_t *)psfd->GetPath());
}

Handle<Value> ClsFileDialog_Dispose(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFileDialog* psfd = (CScriptFileDialog*)self->GetPointerFromInternalField(0);

	if (psfd)
	{
		delete psfd;
		self->SetInternalField(0, External::New(NULL));
	}

	return Undefined();
}

void AddClsFileDialog(Handle<ObjectTemplate> global)
{
	HandleScope handle_scope;

	Handle<FunctionTemplate> cls = FunctionTemplate::New(ClsFileDialogConstructor);

	cls->SetClassName(String::New("CFileDialog"));
	global->Set(String::New("CFileDialog"), cls);
	Handle<ObjectTemplate> proto = cls->PrototypeTemplate();

	proto->Set("Select", FunctionTemplate::New(ClsFileDialog_Select));
	proto->Set("GetPathName", FunctionTemplate::New(ClsFileDialog_GetPathName));
	//proto->Set("Free", FunctionTemplate::New(ClsFileDialog_Free));
	proto->Set("dispose", FunctionTemplate::New(ClsFileDialog_Dispose));

	Handle<ObjectTemplate> inst = cls->InstanceTemplate();

	inst->SetInternalFieldCount(1);

	CScriptBase::AddBaseFunction(inst);
}

#endif //_TA_SNOW_SCRIPT_FILE_DIALOG_H_