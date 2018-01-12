
#ifndef _TA_SNOW_SCRIPT_FILE_H_
#define _TA_SNOW_SCRIPT_FILE_H_
//-----------File for V8Engine--------------

using namespace v8;

Handle<Value> ClsFileConstructor(const Arguments& args)
{
	HandleScope handle_scope;
	Handle<Object> obj = args.This();

	CScriptFile *psf = new CScriptFile;
	obj->SetInternalField(0, External::New(psf));
	return obj;
}

Handle<Value> ClsFile_Open(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);

	if (!psf) return v8::Undefined();

	if (args.Length() > 1)
	{
		Handle<String> path = args[0]->ToString();
		Handle<String> mode = args[1]->ToString();

		if (psf->Open((LPCWSTR)*(String::Value(path)), (LPCWSTR)*(String::Value(mode))))
		{
			return v8::True();
		} else {
			return v8::False();
		}
	}
	return v8::Undefined();
}

Handle<Value> ClsFile_IsOpen(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);

	if (!psf) return v8::Undefined();

	if (psf->IsOpen())
	{
		return v8::True();
	} else {
		return v8::False();
	}
}

Handle<Value> ClsFile_Close(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf) psf->Close();

	return v8::Undefined();
}

Handle<Value> ClsFile_Delete(const Arguments& args)
{
	HandleScope handle_scope;

	//Local<Object> self = args.Holder();
	//CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	//DeleteFile
	if (args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();
		if (DeleteFileW((LPCWSTR)*(String::Value(s))))
		{
			return v8::True();
		} else {
			return v8::False();
		}
	}

	return v8::Undefined();
}

Handle<Value> ClsFile_Move(const Arguments& args)
{
	HandleScope handle_scope;

	//Local<Object> self = args.Holder();
	//CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	//DeleteFile
	if (args.Length() > 1)
	{
		Handle<String> s1 = args[0]->ToString();
		Handle<String> s2 = args[1]->ToString();
		if (MoveFileW((LPCWSTR)*(String::Value(s1)), (LPCWSTR)*(String::Value(s2))))
		{
			return v8::True();
		} else {
			return v8::False();
		}
	}

	return v8::Undefined();
}

Handle<Value> ClsFile_CreateDirectory(const Arguments& args)
{
	HandleScope handle_scope;

	if (args.Length() > 0)
	{
		Handle<String> s1 = args[0]->ToString();
		
		if (CreateDirectoryW((LPCWSTR)*(String::Value(s1)), NULL)) {
			return v8::True();
		} else {
			return v8::False();
		}
	}

	return v8::Undefined();
}

Handle<Value> ClsFile_GetLength(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	//GetLength
	if (psf)
	{
		bool ll = false;

		if (args.Length() > 0)
		{
			ll = args[0]->BooleanValue();
		}
		if (ll)
		{
			double size = psf->GetFileSize64();

			return v8::Number::New(size);
		} else {
			return v8::Uint32::New(psf->GetFileSize());
		}

	} else {
		return v8::Undefined();
	}
}

Handle<Value> ClsFile_Write(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf && args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();
		char *data = NULL;
		lo_W2C(&data, (wchar_t *)*(String::Value(s)));

		int length = lstrlenA(data);

		if (psf->Write((unsigned char *)data, length))
		{
			free(data);
			return v8::True();
		} else {
			free(data);
			return v8::False();
		}
	} else {
		return v8::Undefined();
	}
}

Handle<Value> ClsFile_WriteRaw(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf && args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();

		wchar_t *data = NULL; //utf8ToWcString(*(String::Utf8Value(s)));
		lo_V8S2W(&data, s);

		int length = lstrlenW(data) * sizeof(wchar_t);

		if (psf->Write((unsigned char *)data, length))
		{
			free(data);
			return v8::True();
		} else {
			free(data);
			return v8::False();
		}
	} else {
		return v8::Undefined();
	}
}

Handle<Value> ClsFile_WriteHex(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf && args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();
		char *hexdata = NULL; //::WcToMb((wchar_t *)*(String::Value(s)));
		lo_W2C(&hexdata, (wchar_t *)*(String::Value(s)));

		//LPCWSTR hexdata = *(String::Value(s));
		unsigned char *data = NULL;

		int length = CScriptFile::HexToData(hexdata, &data);
		free(hexdata);
		if (length > 0)
		{
			if (psf->Write(data, length))
			{
				free(data);
				return v8::True();
			} else {
				free(data);
				return v8::False();
			}
		}
	}
		
	return v8::Undefined();
}

#ifdef SCRIPT_FILE_HAVE_DOWNLOAD
Handle<Value> ClsFile_DownloadFrom(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf && args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();
		char *url = NULL; //::WcToMb((wchar_t *)*(String::Value(s)));
		lo_W2C(&url, (wchar_t *)*(String::Value(s)));

		bool bret = psf->DwonloadFrom(url);
		free(url);
		
		if (bret) {
			return v8::True();
		} else {
			return v8::False();
		}
	}
		
	return v8::Undefined();
}
#endif

Handle<Value> ClsFile_Read(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf && args.Length() > 0)
	{
		base::CFileData fd;
		if (fd.Read(*psf))
		{
			std::string utf8text;
			fd.ToUtf8Text(utf8text);

			return String::New(utf8text.c_str());
		}
	}

	return v8::Undefined();
}

/*
Handle<Value> ClsFile_ReadRaw(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf && args.Length() > 0)
	{
		int length = args[0]->Uint32Value();
		unsigned char *data = NULL;

		if (psf->ReadEx(&data, length, true))
		{
			Handle<String> s = String::New((uint16_t *)data);
			free(data);
			return s;
		}
	}

	return v8::Undefined();
}
*/

Handle<Value> ClsFile_ReadHex(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf && args.Length() > 0)
	{
		int length = args[0]->Uint32Value();
		unsigned char *data = NULL;

		if (psf->ReadEx(&data, length))
		{
			char *hexdata = NULL;
			Handle<String> s;
			if (CScriptFile::DataToHex(data, length, &hexdata) > 0)
			{
				s = String::New(hexdata);
				free(hexdata);
			}
			free(data);
			return s;
		}
	}

	return v8::Undefined();
}

Handle<Value> ClsFile_SetPointer(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	int c = args.Length();
	if (psf && c > 0)
	{
		long d = args[0]->Uint32Value();
		unsigned long method = FILE_BEGIN;
		if (c > 1)
		{
			method = args[1]->Uint32Value();
		}
		return v8::Uint32::New(psf->SetPointer(d, method));
	}

	return v8::Undefined();
}

Handle<Value> ClsFile_Dispose(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptFile* psf = (CScriptFile*)self->GetPointerFromInternalField(0);
	
	if (psf)
	{
		delete psf;
		self->SetInternalField(0, External::New(NULL));
	}

	return v8::Undefined();
}

void AddClsFile(Handle<ObjectTemplate> global)
{
	HandleScope handle_scope;

	Handle<FunctionTemplate> cls = FunctionTemplate::New(ClsFileConstructor);

	cls->SetClassName(String::New("CFile"));
	global->Set(String::New("CFile"), cls);
	Handle<ObjectTemplate> proto = cls->PrototypeTemplate();

	proto->Set("Open", FunctionTemplate::New(ClsFile_Open));
	proto->Set("IsOpen", FunctionTemplate::New(ClsFile_IsOpen));
	proto->Set("Close", FunctionTemplate::New(ClsFile_Close));

	proto->Set("Delete", FunctionTemplate::New(ClsFile_Delete));
	proto->Set("Move", FunctionTemplate::New(ClsFile_Move));
	proto->Set("CreateDirectory", FunctionTemplate::New(ClsFile_CreateDirectory));

	proto->Set("Write", FunctionTemplate::New(ClsFile_Write));
	proto->Set("WriteRaw", FunctionTemplate::New(ClsFile_WriteRaw));
	proto->Set("WriteHex", FunctionTemplate::New(ClsFile_WriteHex));
	//proto->Set("WriteString", FunctionTemplate::New(ClsFile_WriteString));

	proto->Set("Read", FunctionTemplate::New(ClsFile_Read));
	//proto->Set("ReadRaw", FunctionTemplate::New(ClsFile_ReadRaw));
	proto->Set("ReadHex", FunctionTemplate::New(ClsFile_ReadHex));
	//proto->Set("ReadLine", FunctionTemplate::New(ClsFile_ReadLine));

	proto->Set("SetPointer", FunctionTemplate::New(ClsFile_SetPointer));

	proto->Set("GetLength", FunctionTemplate::New(ClsFile_GetLength));

#ifdef SCRIPT_FILE_HAVE_DOWNLOAD
	proto->Set("DownloadFrom", FunctionTemplate::New(ClsFile_DownloadFrom));
#endif

	//proto->Set("Free", FunctionTemplate::New(ClsFile_Free));
	proto->Set("dispose", FunctionTemplate::New(ClsFile_Dispose));

	Handle<ObjectTemplate> inst = cls->InstanceTemplate();

	inst->SetInternalFieldCount(1);

	CScriptBase::AddBaseFunction(inst);
}

#endif //_TA_SNOW_SCRIPT_FILE_DIALOG_H_