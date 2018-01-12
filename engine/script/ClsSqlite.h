
#ifndef _TA_SNOW_SCRIPT_SQLITE_H_
#define _TA_SNOW_SCRIPT_SQLITE_H_
//-----------Sqlite for V8Engine--------------

using namespace v8;

Handle<Value> ClsSqliteConstructor(const Arguments& args)
{
	HandleScope handle_scope;
	Handle<Object> obj = args.This();

	CScriptSqlite *psf = new CScriptSqlite;
	obj->SetInternalField(0, External::New(psf));
	return obj;
}

Handle<Value> ClsSqlite_Open(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptSqlite* psf = (CScriptSqlite*)self->GetPointerFromInternalField(0);

	if (!psf) return v8::Undefined();

	if (args.Length() > 0)
	{
		Handle<String> path = args[0]->ToString();

		if (psf->Open((LPCWSTR)*(String::Value(path))))
		{
			return v8::True();
		} else {
			return v8::False();
		}
	}
	return v8::Undefined();
}

Handle<Value> ClsSqlite_IsOpen(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptSqlite* psf = (CScriptSqlite*)self->GetPointerFromInternalField(0);

	if (!psf) return v8::Undefined();

	if (psf->IsOpen())
	{
		return v8::True();
	} else {
		return v8::False();
	}
}

Handle<Value> ClsSqlite_Close(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptSqlite* psf = (CScriptSqlite*)self->GetPointerFromInternalField(0);
	
	if (psf) psf->Close();

	return v8::Undefined();
}

Handle<Value> ClsSqlite_Query(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptSqlite* psf = (CScriptSqlite*)self->GetPointerFromInternalField(0);
	
	if (psf && args.Length() > 0)
	{
		Handle<String> s = args[0]->ToString();

		wchar_t *wdata = NULL;
		char *utf8data = NULL;

		lo_V8S2W(&wdata, s);
		lo_W2Utf8(&utf8data, wdata);
		free(wdata);

		bool bret = psf->Query(utf8data);
		free(utf8data);

		if (bret) {
			return v8::True();
		} else {
			return v8::False();
		}
	} else {
		return v8::Undefined();
	}
}

Handle<Value> ClsSqlite_GetResult(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptSqlite* psf = (CScriptSqlite*)self->GetPointerFromInternalField(0);
	
	
	if (psf) {
		SqliteQueryResult& result = psf->GetResult();
		int col_count = result.GetColCount();
		int row_count = result.GetRowCount();
		if (col_count && row_count) {
			Handle<Array> array = Array::New(row_count);
			Handle<String> *cols = new Handle<String>[col_count];

			int icol, irow;

			for (icol = 0; icol < col_count; icol++) {
				cols[icol] = String::New(result.GetColName(icol));
			}

			for (irow = 0; irow < row_count; irow++) {
				Handle<Object> rowobj = Object::New();
				for (icol = 0; icol < col_count; icol++) {
					sqlite_row_data rwodata;
					rwodata.uint64_ = 0;

					if (result.GetRowData(irow, icol, &rwodata)) {
						switch (rwodata.type) {
						case kCTString:
							rowobj->Set(cols[icol], String::New(rwodata.string_ ? rwodata.string_ : ""));
							break;
						case kCTBLOB:
							if (rwodata.blob_) {
								rowobj->Set(cols[icol], String::New((char *)rwodata.blob_, rwodata.byteslen));
							} else {
								rowobj->Set(cols[icol], v8::Null());
							}
							break;
						case kCTInteger:
							rowobj->Set(cols[icol], v8::Integer::New(rwodata.int64_));
							break;
						case kCTFloat:
							rowobj->Set(cols[icol], v8::Number::New(rwodata.double_));
							break;
						case kCTNULL:
							rowobj->Set(cols[icol], v8::Null());
							break;
						default:
							rowobj->Set(cols[icol], v8::Undefined());
							break;
						}
					} else {
						rowobj->Set(cols[icol], v8::Undefined());
					}
				}

				array->Set(Integer::New(irow), rowobj);
			}

			delete[] cols;

			return array;
		}
	}
	return v8::Undefined();
}

Handle<Value> ClsSqlite_GetErrorMessage(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptSqlite* psf = (CScriptSqlite*)self->GetPointerFromInternalField(0);
	
	if (psf) {
		return v8::String::New((uint16_t *)psf->GetErrorMessage());
	}
	return v8::Undefined();
}

Handle<Value> ClsSqlite_Dispose(const Arguments& args)
{
	HandleScope handle_scope;

	Local<Object> self = args.Holder();
	CScriptSqlite* psf = (CScriptSqlite*)self->GetPointerFromInternalField(0);
	
	if (psf)
	{
		delete psf;
		self->SetInternalField(0, External::New(NULL));
	}

	return v8::Undefined();
}

void AddClsSqlite(Handle<ObjectTemplate> global)
{
	HandleScope handle_scope;

	Handle<FunctionTemplate> cls = FunctionTemplate::New(ClsSqliteConstructor);

	CScriptSqlite::Init();

	cls->SetClassName(String::New("CSqlite"));
	global->Set(String::New("CSqlite"), cls);
	Handle<ObjectTemplate> proto = cls->PrototypeTemplate();

	proto->Set("Open", FunctionTemplate::New(ClsSqlite_Open));
	proto->Set("IsOpen", FunctionTemplate::New(ClsSqlite_IsOpen));
	proto->Set("Close", FunctionTemplate::New(ClsSqlite_Close));

	proto->Set("Query", FunctionTemplate::New(ClsSqlite_Query));
	proto->Set("GetResult", FunctionTemplate::New(ClsSqlite_GetResult));

	proto->Set("GetErrorMessage", FunctionTemplate::New(ClsSqlite_GetErrorMessage));

	//proto->Set("Free", FunctionTemplate::New(ClsSqlite_Free));
	proto->Set("dispose", FunctionTemplate::New(ClsSqlite_Dispose));

	Handle<ObjectTemplate> inst = cls->InstanceTemplate();

	inst->SetInternalFieldCount(1);

	CScriptBase::AddBaseFunction(inst);
}

#endif //_TA_SNOW_SCRIPT_FILE_DIALOG_H_