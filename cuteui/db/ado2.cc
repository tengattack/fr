// 
//   MODULE: Ado2.cc
//
//   Author: tengattack
//

#include "base/common.h"
#include "base/string/stringprintf.h"
#include "base/string/utf_string_conversions.h"
#include "ado2.h"

#define ASSERT
#define TRACE
#pragma warning(push)
#pragma warning(disable: 4146)
#pragma warning(disable: 4244)	//close x64 long to x86 long warning

#define  ChunkSize 100

//------------------------//
// Convert char * to BSTR //
//------------------------//
BSTR ConvertStringToBSTR(std::wstring& str)
{
    DWORD cwch = str.length();
    BSTR wsOut(NULL);

    wsOut = ::SysAllocStringLen(NULL, cwch);
    if(wsOut)
	{
		memcpy(wsOut, str.c_str(), cwch * sizeof(wchar_t));
	}
	//::SysFreeString(wsOut);//must clean up
    return wsOut;
}

#ifdef C_JET_ENGINE_IMPORT
bool CJetEngine::CCompactDatabase(std::wstring strDatabaseSource, std::wstring strDatabaseDestination)
{
     try 
    {
        ::CoInitialize(NULL);
        IJetEnginePtr jet(__uuidof(JetEngine));        
        HRESULT hr = jet->CompactDatabase(_bstr_t(strDatabaseSource.GetBuffer(0)), _bstr_t(strDatabaseDestination.GetBuffer(0)));
        ::CoUninitialize();
         return hr == S_OK;
    }
     catch (_com_error& e) 
    {       
        ::CoUninitialize();
         return false;
    } 
}
#endif

////////////////////////////////////////////////////// / 
// 
//  CADODatabase Class
//

DWORD CADODatabase::GetRecordCount(_RecordsetPtr m_pRs)
{
    DWORD numRows =  0 ;
    
    numRows = m_pRs->GetRecordCount();

     if (numRows ==  - 1)
    {
         if (m_pRs->EndOfFile != VARIANT_TRUE)
            m_pRs->MoveFirst();

         while (m_pRs->EndOfFile != VARIANT_TRUE)
        {
            numRows ++ ;
            m_pRs->MoveNext();
        }
         if (numRows  >   0)
            m_pRs->MoveFirst();
    }
     return numRows;
}

bool CADODatabase::Create(LPCTSTR lpstrFileName)
{
	HRESULT hr = S_OK;
	std::wstring strConn;
	base::SStringPrintf(&strConn, L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=\"%s\";", lpstrFileName);

	try
	{     
		ADOX::_CatalogPtr m_pCatalog = NULL;
		hr = m_pCatalog.CreateInstance(__uuidof (ADOX::Catalog));

		if(FAILED(hr))
		{  
			_com_issue_error(hr);
		}else{
			m_pCatalog->Create(_bstr_t(strConn.c_str())); //Create MDB
		}
		return hr == S_OK;
	}
	catch(_com_error& e)
	{
		// Notify the user of errors if any.
		dump_com_error(e);
		return false;
		//cout << "create error" << endl;
	}
}

bool CADODatabase::Open(LPCTSTR lpstrConnection, LPCTSTR lpstrUserID, LPCTSTR lpstrPassword)
{
    HRESULT hr = S_OK;

     if (IsOpen())
        Close();

     if (lstrcmp(lpstrConnection, _T("")) != 0)
        m_strConnection = lpstrConnection;

    ASSERT(!m_strConnection.empty());

    try 
    {
        if (m_nConnectionTimeout != 0)
            m_pConnection->PutConnectionTimeout(m_nConnectionTimeout);
		hr = m_pConnection->Open(_bstr_t(m_strConnection.c_str()), _bstr_t(lpstrUserID), _bstr_t(lpstrPassword), NULL);
        return hr == S_OK;
    }
	catch (_com_error& e)
    {
        dump_com_error(e);
        return false;
    }
    
}

void CADODatabase::dump_com_error(_com_error& e)
{
    std::wstring ErrorStr;
    
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
	base::SStringPrintf(&ErrorStr, L"CADODataBase Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n",
        e.Error(), e.ErrorMessage(), (LPCTSTR)bstrSource, (LPCTSTR)bstrDescription);
    m_strErrorDescription = (LPCTSTR)bstrDescription ;
    
	base::SStringPrintf(&m_strLastError, L"Connection String = %s\r\n%s", GetConnectionString().c_str(), ErrorStr.c_str());
    m_dwLastError = e.Error();

    #ifdef _DEBUG
	MessageBox(NULL, ErrorStr.c_str(), L"Error", MB_OK | MB_ICONERROR);
    #endif 
}

bool CADODatabase::IsOpen()
{
     if (m_pConnection)
         return m_pConnection->GetState() != adStateClosed;
     return false;
}

void CADODatabase::Close()
{
     if (IsOpen())
        m_pConnection->Close();
}


////////////////////////////////////////////////////// / 
// 
//  CADORecordset Class
//

CADORecordset::CADORecordset()
{
    m_pRecordset = NULL;
    m_pCmd = NULL;
    m_strQuery = _T("");
    m_strLastError = _T("");
    m_dwLastError =  0 ;
    m_pRecBinding = NULL;
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    m_pCmd.CreateInstance(__uuidof(Command));
    m_nEditStatus = CADORecordset::dbEditNone;
    m_nSearchDirection = CADORecordset::searchForward;
}

CADORecordset::CADORecordset(CADODatabase *  pAdoDatabase)
{
    m_pRecordset = NULL;
    m_pCmd = NULL;
    m_strQuery = _T("");
    m_strLastError = _T("");
    m_dwLastError =  0 ;
    m_pRecBinding = NULL;
    m_pRecordset.CreateInstance(__uuidof(Recordset));
    m_pCmd.CreateInstance(__uuidof(Command));
    m_nEditStatus = CADORecordset::dbEditNone;
    m_nSearchDirection = CADORecordset::searchForward;

    m_pConnection = pAdoDatabase->GetActiveConnection();
}

bool CADORecordset::Open(_ConnectionPtr mpdb, LPCTSTR lpstrExec, int nOption)
{    
    Close();
    
     if (lstrcmp(lpstrExec, _T("")) != 0)
        m_strQuery = lpstrExec;

    ASSERT(!m_strQuery.empty());

    if (m_pConnection == NULL)
        m_pConnection = mpdb;

    //m_strQuery.TrimLeft();
	//m_strQuery.compare(7, L"Select ");
	
    bool bIsSelect = //m_strQuery.Mid(0, strlen("Select ")).CompareNoCase("select ") == 0 && nOption == openUnknown;
		::_wcsnicmp(m_strQuery.c_str(), L"select ", 7) == 0 && nOption == openUnknown;

     try 
    {
        m_pRecordset->CursorType = adOpenStatic;
        m_pRecordset->CursorLocation = adUseClient;
        if (bIsSelect  ||  nOption == openQuery  ||  nOption == openUnknown)
			 m_pRecordset->Open(m_strQuery.c_str(), _variant_t((IDispatch *)mpdb, true), 
                            adOpenStatic, adLockOptimistic, adCmdUnknown);
        else if (nOption == openTable)
            m_pRecordset->Open(m_strQuery.c_str(), _variant_t((IDispatch *)mpdb, true), 
                            adOpenKeyset, adLockOptimistic, adCmdTable);
        else if (nOption == openStoredProc)
        {
            m_pCmd->ActiveConnection = mpdb;
            m_pCmd->CommandText = _bstr_t(m_strQuery.c_str());
            m_pCmd->CommandType = adCmdStoredProc;
            m_pConnection->CursorLocation = adUseClient;
            
            m_pRecordset = m_pCmd->Execute(NULL, NULL, adCmdText);
        }
        else 
        {
            TRACE("Unknown parameter. %d", nOption);
            return false;
        }
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }

     return m_pRecordset != NULL;
}

bool CADORecordset::Open(LPCTSTR lpstrExec, int nOption)
{
    ASSERT(m_pConnection != NULL);
    ASSERT(m_pConnection->GetState() != adStateClosed);
     return Open(m_pConnection, lpstrExec, nOption);
}

bool CADORecordset::OpenSchema(int nSchema, LPCTSTR SchemaID)
{
     try 
    {
        _variant_t vtSchemaID = vtMissing;

         if (lstrlen(SchemaID) != 0)
            vtSchemaID = SchemaID;
            
        m_pRecordset = m_pConnection->OpenSchema((enum  SchemaEnum)nSchema, vtMissing, vtSchemaID);
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::Requery()
{
     if (IsOpen())
    {
         try 
        {
            m_pRecordset->Requery(adExecuteRecord);
        }
         catch (_com_error& e)
        {
            dump_com_error(e);
             return false;
        }
    }
     return true;
}


bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, double &  dbValue)
{    
     double  val = (double)NULL;
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         switch (vtFld.vt)
        {
         case VT_R4:
            val = vtFld.fltVal;
             break;
         case VT_R8:
            val = vtFld.dblVal;
             break;
         case VT_DECIMAL:
             // Corrected by Jos?Carlos Martínez Galán 
            val = vtFld.decVal.Lo32;
            val  *=  (vtFld.decVal.sign ==  128) ?   - 1  :  1 ;
            val  /=  pow((double)10 , vtFld.decVal.scale); 
             break;
         case VT_UI1:
            val = vtFld.iVal;
             break;
         case VT_I2:
         case VT_I4:
            val = vtFld.lVal;
             break;
         case VT_INT:
            val = vtFld.intVal;
             break;
         case VT_NULL:
         case VT_EMPTY:
            val =  0 ;
             break;
         default:
            val = vtFld.dblVal;
        }
        dbValue = val;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}


bool CADORecordset::GetFieldValue(int nIndex, double &  dbValue)
{    
     double  val = (double)NULL;
    _variant_t vtFld;
    _variant_t vtIndex;

    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         switch (vtFld.vt)
        {
         case VT_R4:
            val = vtFld.fltVal;
             break;
         case VT_R8:
            val = vtFld.dblVal;
             break;
         case VT_DECIMAL:
             // Corrected by Jos?Carlos Martínez Galán 
            val = vtFld.decVal.Lo32;
            val  *=  (vtFld.decVal.sign ==  128) ?   - 1  :  1 ;
            val  /=  pow((double)10 , vtFld.decVal.scale); 
             break;
         case VT_UI1:
            val = vtFld.iVal;
             break;
         case VT_I2:
         case VT_I4:
            val = vtFld.lVal;
             break;
         case VT_INT:
            val = vtFld.intVal;
             break;
         case VT_NULL:
         case VT_EMPTY:
            val =  0 ;
             break;
         default:
            val =  0 ;
        }
        dbValue = val;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}


bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, long &  lValue)
{
     long val = (long)NULL;
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         if (vtFld.vt != VT_NULL  &&  vtFld.vt != VT_EMPTY)
            val = vtFld.lVal;
        lValue = val;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(int nIndex, long &  lValue)
{
     long val = (long)NULL;
    _variant_t vtFld;
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;

     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         if (vtFld.vt != VT_NULL  &&  vtFld.vt != VT_EMPTY)
            val = vtFld.lVal;
        lValue = val;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}


bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, unsigned  long &  ulValue)
{
     long val = (long)NULL;
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         if (vtFld.vt != VT_NULL  &&  vtFld.vt != VT_EMPTY)
            val = vtFld.ulVal;
        ulValue = val;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(int nIndex, unsigned  long &  ulValue)
{
     long val = (long)NULL;
    _variant_t vtFld;
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;

     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         if (vtFld.vt != VT_NULL  &&  vtFld.vt != VT_EMPTY)
            val = vtFld.ulVal;
        ulValue = val;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, int& nValue)
{
     int val = NULL;
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         switch (vtFld.vt)
        {
         case VT_BOOL:
            val = vtFld.boolVal;
             break;
         case VT_I2:
         case VT_UI1:
            val = vtFld.iVal;
             break;
         case VT_INT:
            val = vtFld.intVal;
             break;
         case VT_NULL:
         case VT_EMPTY:
            val =  0 ;
             break;
         default:
            val = vtFld.iVal;
        }    
        nValue = val;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(int nIndex, int& nValue)
{
     int val = (int)NULL;
    _variant_t vtFld;
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;

     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         switch (vtFld.vt)
        {
         case VT_BOOL:
            val = vtFld.boolVal;
             break;
         case VT_I2:
         case VT_UI1:
            val = vtFld.iVal;
             break;
         case VT_INT:
            val = vtFld.intVal;
             break;
         case VT_NULL:
         case VT_EMPTY:
            val =  0 ;
             break;
         default:
            val = vtFld.iVal;
        }    
        nValue = val;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, std::wstring& strValue, std::wstring strDateFormat)
{
    std::wstring str = _T("");
    _variant_t vtFld;

     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         switch (vtFld.vt) 
        {
         case VT_R4:
            str = DblToStr(vtFld.fltVal);
             break;
         case VT_R8:
            str = DblToStr(vtFld.dblVal);
             break;
         case VT_BSTR:
            str = vtFld.bstrVal;
             break;
         case VT_I2:
         case VT_UI1:
            str = IntToStr(vtFld.iVal);
             break;
         case VT_INT:
            str = IntToStr(vtFld.intVal);
             break;
         case VT_I4:
            str = LongToStr(vtFld.lVal);
             break;
         case VT_UI4:
            str = ULongToStr(vtFld.ulVal);
             break;
         case VT_DECIMAL:
            {
             // Corrected by Jos?Carlos Martínez Galán 
             double  val = vtFld.decVal.Lo32;
            val  *=  (vtFld.decVal.sign ==  128) ?   - 1  :  1 ;
            val  /=  pow((double)10 , vtFld.decVal.scale); 
            str = DblToStr(val);
            }
             break;
         case VT_DATE:
            {
                /*COleDateTime dt(vtFld);

                if (strDateFormat.empty())
                    strDateFormat = _T("%Y-%m-%d %H:%M:%S");
                str = dt.Format(strDateFormat);*/
				//Date stamp
				str = base::UintToString16(vtFld);
            }
             break;
         case VT_EMPTY:
         case VT_NULL:
            str.clear();
             break;
         case VT_BOOL:
            str = vtFld.boolVal == VARIANT_TRUE ?  'T':'F';
             break;
         default:
            str.clear();
             return false;
        }
        strValue = str;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(int nIndex, std::wstring& strValue, std::wstring strDateFormat)
{
    std::wstring str = _T("");
    _variant_t vtFld;
    _variant_t vtIndex;

    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         switch (vtFld.vt) 
        {
         case VT_R4:
            str = DblToStr(vtFld.fltVal);
             break;
         case VT_R8:
            str = DblToStr(vtFld.dblVal);
             break;
         case VT_BSTR:
            str = vtFld.bstrVal;
             break;
         case VT_I2:
         case VT_UI1:
            str = IntToStr(vtFld.iVal);
             break;
         case VT_INT:
            str = IntToStr(vtFld.intVal);
             break;
         case VT_I4:
            str = LongToStr(vtFld.lVal);
             break;
         case VT_UI4:
            str = ULongToStr(vtFld.ulVal);
             break;
         case VT_DECIMAL:
            {
             // Corrected by Jos?Carlos Martínez Galán 
             double  val = vtFld.decVal.Lo32;
            val  *=  (vtFld.decVal.sign ==  128) ?   - 1  :  1 ;
            val  /=  pow((double)10 , vtFld.decVal.scale); 
            str = DblToStr(val);
            }
             break;
         case VT_DATE:
            {
                /*COleDateTime dt(vtFld);
                
                 if (strDateFormat.empty())
                    strDateFormat = _T("%Y-%m-%d %H:%M:%S");
                str = dt.Format(strDateFormat);*/
				//Date stamp
				str = base::UintToString16(vtFld);
            }
             break;
         case VT_BOOL:
            str = vtFld.boolVal == VARIANT_TRUE ?  'T':'F';
             break;
         case VT_EMPTY:
         case VT_NULL:
            str.clear();
             break;
         default:
            str.clear();
             return false;
        }
        strValue = str;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

/*bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, COleDateTime &  time)
{
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         switch (vtFld.vt) 
        {
         case VT_DATE:
            {
                COleDateTime dt(vtFld);
                time = dt;
            }
             break;
         case VT_EMPTY:
         case VT_NULL:
            time.SetStatus(COleDateTime:: null);
             break;
         default:
             return false;
        }
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(int nIndex, COleDateTime &  time)
{
    _variant_t vtFld;
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         switch (vtFld.vt) 
        {
         case VT_DATE:
            {
                COleDateTime dt(vtFld);
                time = dt;
            }
             break;
         case VT_EMPTY:
         case VT_NULL:
            time.SetStatus(COleDateTime:: null);
             break;
         default:
             return false;
        }
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}*/

bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, bool& bValue)
{
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         switch (vtFld.vt) 
        {
         case VT_BOOL:
            bValue = vtFld.boolVal == VARIANT_TRUE ?   true :  false ;
             break;
         case VT_EMPTY:
         case VT_NULL:
            bValue =  false ;
             break;
         default:
             return false;
        }
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(int nIndex, bool& bValue)
{
    _variant_t vtFld;
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         switch (vtFld.vt) 
        {
         case VT_BOOL:
            bValue = vtFld.boolVal == VARIANT_TRUE ?   true :  false ;
             break;
         case VT_EMPTY:
         case VT_NULL:
            bValue =  false ;
             break;
         default:
             return false;
        }
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

/*bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, COleCurrency &  cyValue)
{
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         switch (vtFld.vt) 
        {
         case VT_CY:
            cyValue = (CURRENCY)vtFld.cyVal;
             break;
         case VT_EMPTY:
         case VT_NULL:
            {
            cyValue = COleCurrency();
            cyValue.m_status = COleCurrency:: null ;
            }
             break;
         default:
             return false;
        }
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(int nIndex, COleCurrency &  cyValue)
{
    _variant_t vtFld;
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         switch (vtFld.vt) 
        {
         case VT_CY:
            cyValue = (CURRENCY)vtFld.cyVal;
             break;
         case VT_EMPTY:
         case VT_NULL:
            {
            cyValue = COleCurrency();
            cyValue.m_status = COleCurrency:: null ;
            }
             break;
         default:
             return false;
        }
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}*/

bool CADORecordset::GetFieldValue(LPCTSTR lpFieldName, _variant_t &  vtValue)
{
     try 
    {
        vtValue = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::GetFieldValue(int nIndex, _variant_t &  vtValue)
{
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     try 
    {
        vtValue = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::IsFieldNull(LPCTSTR lpFieldName)
{
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         return vtFld.vt == VT_NULL;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::IsFieldNull(int nIndex)
{
    _variant_t vtFld;
    _variant_t vtIndex;

    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         return vtFld.vt == VT_NULL;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::IsFieldEmpty(LPCTSTR lpFieldName)
{
    _variant_t vtFld;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
         return vtFld.vt == VT_EMPTY  ||  vtFld.vt == VT_NULL;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::IsFieldEmpty(int nIndex)
{
    _variant_t vtFld;
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     try 
    {
        vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
         return vtFld.vt == VT_EMPTY  ||  vtFld.vt == VT_NULL;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::SetFieldEmpty(LPCTSTR lpFieldName)
{
    _variant_t vtFld;
    vtFld.vt = VT_EMPTY;
    
     return PutFieldValue(lpFieldName, vtFld);
}

bool CADORecordset::SetFieldEmpty(int nIndex)
{
    _variant_t vtFld;
    vtFld.vt = VT_EMPTY;

    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     return PutFieldValue(vtIndex, vtFld);
}


DWORD CADORecordset::GetRecordCount()
{
    DWORD nRows =  0 ;
    
    nRows = m_pRecordset->GetRecordCount();

     if (nRows ==  - 1)
    {
        nRows =  0 ;
         if (m_pRecordset->EndOfFile != VARIANT_TRUE)
            m_pRecordset->MoveFirst();
        
         while (m_pRecordset->EndOfFile != VARIANT_TRUE)
        {
            nRows ++ ;
            m_pRecordset->MoveNext();
        }
         if (nRows  >   0)
            m_pRecordset->MoveFirst();
    }
    
     return nRows;
}

bool CADORecordset::IsOpen()
{
     if (m_pRecordset != NULL  &&  IsConnectionOpen())
         return m_pRecordset->GetState() != adStateClosed;
     return false;
}

void CADORecordset::Close()
{
     if (IsOpen())
    {
         if  (m_nEditStatus != dbEditNone)
              CancelUpdate();

        m_pRecordset->PutSort(_T(""));
        m_pRecordset->Close();    
    }
}


bool CADODatabase::Execute(LPCTSTR lpstrExec)
{
    ASSERT(m_pConnection != NULL);
    ASSERT(lstrcmp(lpstrExec, _T("")) != 0);
    _variant_t vRecords;
    
    m_nRecordsAffected =  0 ;

     try 
    {
        m_pConnection->CursorLocation = adUseClient;
        m_pConnection->Execute(_bstr_t(lpstrExec), & vRecords, adExecuteNoRecords);
        m_nRecordsAffected = vRecords.iVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;    
    }
}

bool CADORecordset::RecordBinding(CADORecordBinding  & pAdoRecordBinding)
{
    HRESULT hr;
    m_pRecBinding = NULL;

     // Open the binding interface. 
     if (FAILED(hr = m_pRecordset->QueryInterface(__uuidof(IADORecordBinding), (LPVOID *) & m_pRecBinding)))
    {
        _com_issue_error(hr);
         return false;
    }
    
     // Bind the recordset to class 
     if (FAILED(hr = m_pRecBinding->BindToRecordset(& pAdoRecordBinding)))
    {
        _com_issue_error(hr);
         return false;
    }
     return true;
}

bool CADORecordset::GetFieldInfo(LPCTSTR lpFieldName, CADOFieldInfo *  fldInfo)
{
    FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);
    
     return GetFieldInfo(pField, fldInfo);
}

bool CADORecordset::GetFieldInfo(int nIndex, CADOFieldInfo *  fldInfo)
{
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;

    FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

     return GetFieldInfo(pField, fldInfo);
}


bool CADORecordset::GetFieldInfo(FieldPtr pField, CADOFieldInfo* fldInfo)
{

    fldInfo->m_strName = (LPCTSTR)pField->GetName();
    fldInfo->m_lDefinedSize = pField->GetDefinedSize();
    fldInfo->m_nType = pField->GetType();
    fldInfo->m_lAttributes = pField->GetAttributes();
    if (!IsEof())
		fldInfo->m_lSize = pField->GetActualSize();
	else
		fldInfo->m_lSize = 0;
    return true;
}

bool CADORecordset::GetChunk(LPCTSTR lpFieldName, std::wstring& strValue)
{
    FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);
    
    return GetChunk(pField, strValue);
}

bool CADORecordset::GetChunk(int nIndex, std::wstring& strValue)
{
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;

    FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);
    
    return GetChunk(pField, strValue);
}


bool CADORecordset::GetChunk(FieldPtr pField, std::wstring& strValue)
{
    std::wstring str = _T("");
     long lngSize, lngOffSet =  0 ;
    _variant_t varChunk;

    lngSize = pField->ActualSize;
    
    str.clear();
     while (lngOffSet  <  lngSize)
    { 
         try 
        {
            varChunk = pField->GetChunk(ChunkSize);
            
            str  +=  varChunk.bstrVal;
            lngOffSet  +=  ChunkSize;
        }
         catch (_com_error& e)
        {
            dump_com_error(e);
             return false;
        }
    }

    lngOffSet =  0 ;
    strValue = str;
     return true;
}

bool CADORecordset::GetChunk(LPCTSTR lpFieldName, LPVOID lpData)
{
    FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);

     return GetChunk(pField, lpData);
}

bool CADORecordset::GetChunk(int nIndex, LPVOID lpData)
{
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;

    FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

     return GetChunk(pField, lpData);
}

bool CADORecordset::GetChunk(FieldPtr pField, LPVOID lpData)
{
     long lngSize, lngOffSet =  0 ;
    _variant_t varChunk;    
    UCHAR chData;
    HRESULT hr;
     long lBytesCopied =  0 ;

    lngSize = pField->ActualSize;
    
     while (lngOffSet  <  lngSize)
    { 
         try 
        {
            varChunk = pField->GetChunk(ChunkSize);

             // Copy the data only upto the Actual Size of Field.   
             for (long lIndex =  0 ; lIndex  <=  (ChunkSize  -   1); lIndex ++)
            {
                hr =  SafeArrayGetElement(varChunk.parray, & lIndex, & chData);
                 if (SUCCEEDED(hr))
                {
                     // Take BYTE by BYTE and advance Memory Location
                     // hr = SafeArrayPutElement((SAFEARRAY FAR*)lpData, &lBytesCopied ,&chData);  
                    ((UCHAR *)lpData)[lBytesCopied] = chData;
                    lBytesCopied ++ ;
                }
                 else 
                     break;
            }
            lngOffSet  +=  ChunkSize;
        }
         catch (_com_error& e)
        {
            dump_com_error(e);
             return false;
        }
    }

    lngOffSet =  0 ;
     return true;
}

bool CADORecordset::AppendChunk(LPCTSTR lpFieldName, LPVOID lpData, UINT nBytes)
{

    FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);

     return AppendChunk(pField, lpData, nBytes);
}


bool CADORecordset::AppendChunk(int nIndex, LPVOID lpData, UINT nBytes)
{
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;

    FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

     return AppendChunk(pField, lpData, nBytes);
}

bool CADORecordset::AppendChunk(FieldPtr pField, LPVOID lpData, UINT nBytes)
{
    HRESULT hr;
    _variant_t varChunk;
     long lngOffset =  0 ;
    UCHAR chData;
    SAFEARRAY FAR  * psa = NULL;
    SAFEARRAYBOUND rgsabound[ 1 ];

     try 
    {
         // Create a safe array to store the array of BYTES  
        rgsabound[ 0 ].lLbound =  0 ;
        rgsabound[ 0 ].cElements = nBytes;
        psa = SafeArrayCreate(VT_UI1, 1 ,rgsabound);

         while (lngOffset  <  (long)nBytes)
        {
            chData    = ((UCHAR *)lpData)[lngOffset];
            hr = SafeArrayPutElement(psa, & lngOffset, & chData);

             if (FAILED(hr))
                 return false;
            
            lngOffset ++ ;
        }
        lngOffset =  0 ;

         // Assign the Safe array  to a variant.  
        varChunk.vt = VT_ARRAY | VT_UI1;
        varChunk.parray = psa;

        hr = pField->AppendChunk(varChunk);

         if (SUCCEEDED(hr))  return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }

     return false;
}

std::wstring CADORecordset::GetString(LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows)
{
    _bstr_t varOutput;
    _bstr_t varNull("");
    _bstr_t varCols("\t");
    _bstr_t varRows("\r");

     if (lstrlen(lpCols) != 0)
        varCols = _bstr_t(lpCols);

     if (lstrlen(lpRows) != 0)
        varRows = _bstr_t(lpRows);
    
     if (numRows ==  0)
        numRows  = (long)GetRecordCount();            
            
    varOutput = m_pRecordset->GetString(adClipString, numRows, varCols, varRows, varNull);

    return (LPCTSTR)varOutput;
}

std::wstring IntToStr(int nVal)
{
    return base::IntToString16(nVal);
}

std::wstring LongToStr(long lVal)
{
    return base::IntToString16(lVal);
}

std::wstring ULongToStr(unsigned  long ulVal)
{
	return base::UintToString16(ulVal);
}


std::wstring DblToStr(double  dblVal, int ndigits)
{
	char buff[50];
	_gcvt(dblVal, ndigits, buff);
	
	return ASCIIToWide(buff);
}

std::wstring DblToStr(float  fltVal)
{
	char buff[50];
	_gcvt(fltVal, 10 , buff);

	return ASCIIToWide(buff);
}

void CADORecordset::Edit()
{
    m_nEditStatus = dbEdit;
}

bool CADORecordset::AddNew()
{
    m_nEditStatus = dbEditNone;
     if (m_pRecordset->AddNew() != S_OK)
         return false;

    m_nEditStatus = dbEditNew;
     return true;
}

bool CADORecordset::AddNew(CADORecordBinding  & pAdoRecordBinding)
{
     try 
    {
         if (m_pRecBinding->AddNew(& pAdoRecordBinding) != S_OK)
        {
             return false;
        }
         else 
        {
            m_pRecBinding->Update(& pAdoRecordBinding);
             return true;
        }
            
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }    
}

bool CADORecordset::Update()
{
    bool bret = true;

     if (m_nEditStatus != dbEditNone)
    {

         try 
        {
             if (m_pRecordset->Update() != S_OK)
                bret = false;
        }
         catch (_com_error& e)
        {
            dump_com_error(e);
            bret = false;
        }

         if (!bret)
            m_pRecordset->CancelUpdate();
        m_nEditStatus = dbEditNone;
    }
     return bret;
}

void CADORecordset::CancelUpdate()
{
    m_pRecordset->CancelUpdate();
    m_nEditStatus = dbEditNone;
}

bool CADORecordset::SetFieldValue(int nIndex, std::wstring strValue)
{
	_variant_t vtFld;
	_variant_t vtIndex;    
    
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	if (!strValue.empty())
		vtFld.vt = VT_BSTR;
	else 
		vtFld.vt = VT_NULL;

	//strValue.AllocSysString()
	vtFld.bstrVal = ::ConvertStringToBSTR(strValue);
	bool b_ret = PutFieldValue(vtIndex, vtFld);
	//::SysFreeString(vtFld.bstrVal);
	return b_ret;
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, std::wstring strValue)
{
	_variant_t vtFld;

	if (!strValue.empty())
		vtFld.vt = VT_BSTR;
	else 
		vtFld.vt = VT_NULL;

    //strValue.AllocSysString();
	vtFld.bstrVal = ::ConvertStringToBSTR(strValue);
	bool b_ret = PutFieldValue(lpFieldName, vtFld);
	//::SysFreeString(vtFld.bstrVal);
	return b_ret;
}

bool CADORecordset::SetFieldValue(int nIndex, int nValue)
{
    _variant_t vtFld;
    
    vtFld.vt = VT_I2;
    vtFld.iVal = nValue;
    
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     return PutFieldValue(vtIndex, vtFld);
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, int nValue)
{
    _variant_t vtFld;
    
    vtFld.vt = VT_I2;
    vtFld.iVal = nValue;
    
    
     return PutFieldValue(lpFieldName, vtFld);
}

bool CADORecordset::SetFieldValue(int nIndex, long lValue)
{
    _variant_t vtFld;
    vtFld.vt = VT_I4;
    vtFld.lVal = lValue;
    
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     return PutFieldValue(vtIndex, vtFld);
    
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, long lValue)
{
    _variant_t vtFld;
    vtFld.vt = VT_I4;
    vtFld.lVal = lValue;
    
     return PutFieldValue(lpFieldName, vtFld);
}

bool CADORecordset::SetFieldValue(int nIndex, unsigned  long ulValue)
{
    _variant_t vtFld;
    vtFld.vt = VT_UI4;
    vtFld.ulVal = ulValue;
    
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     return PutFieldValue(vtIndex, vtFld);
    
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, unsigned  long ulValue)
{
    _variant_t vtFld;
    vtFld.vt = VT_UI4;
    vtFld.ulVal = ulValue;
    
     return PutFieldValue(lpFieldName, vtFld);
}

bool CADORecordset::SetFieldValue(int nIndex, double  dblValue)
{
    _variant_t vtFld;
    vtFld.vt = VT_R8;
    vtFld.dblVal = dblValue;

    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;

     return PutFieldValue(vtIndex, vtFld);
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, double  dblValue)
{
    _variant_t vtFld;
    vtFld.vt = VT_R8;
    vtFld.dblVal = dblValue;
        
     return PutFieldValue(lpFieldName, vtFld);
}

/*bool CADORecordset::SetFieldValue(int nIndex, COleDateTime time)
{
    _variant_t vtFld;
    vtFld.vt = VT_DATE;
    vtFld.date = time;
    
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     return PutFieldValue(vtIndex, vtFld);
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, COleDateTime time)
{
    _variant_t vtFld;
    vtFld.vt = VT_DATE;
    vtFld.date = time;
    
     return PutFieldValue(lpFieldName, vtFld);
}*/



bool CADORecordset::SetFieldValue(int nIndex, bool  bValue)
{
    _variant_t vtFld;
    vtFld.vt = VT_BOOL;
    vtFld.boolVal = bValue;
    
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     return PutFieldValue(vtIndex, vtFld);
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, bool  bValue)
{
    _variant_t vtFld;
    vtFld.vt = VT_BOOL;
    vtFld.boolVal = bValue;
    
     return PutFieldValue(lpFieldName, vtFld);
}


/*bool CADORecordset::SetFieldValue(int nIndex, COleCurrency cyValue)
{
     if (cyValue.m_status == COleCurrency::invalid)
         return false;

    _variant_t vtFld;
        
    vtFld.vt = VT_CY;
    vtFld.cyVal = cyValue.m_cur;
    
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     return PutFieldValue(vtIndex, vtFld);
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, COleCurrency cyValue)
{
     if (cyValue.m_status == COleCurrency::invalid)
         return false;

    _variant_t vtFld;

    vtFld.vt = VT_CY;
    vtFld.cyVal = cyValue.m_cur;    
        
     return PutFieldValue(lpFieldName, vtFld);
}*/

bool CADORecordset::SetFieldValue(int nIndex, _variant_t vtValue)
{
    _variant_t vtIndex;
    
    vtIndex.vt = VT_I2;
    vtIndex.iVal = nIndex;
    
     return PutFieldValue(vtIndex, vtValue);
}

bool CADORecordset::SetFieldValue(LPCTSTR lpFieldName, _variant_t vtValue)
{    
     return PutFieldValue(lpFieldName, vtValue);
}


bool CADORecordset::SetBookmark()
{
     if (m_varBookmark.vt != VT_EMPTY)
    {
        m_pRecordset->Bookmark = m_varBookmark;
         return true;
    }
     return false;
}

bool CADORecordset::Delete()
{
     if (m_pRecordset->Delete(adAffectCurrent) != S_OK)
         return false;

     if (m_pRecordset->Update() != S_OK)
         return false;
    
    m_nEditStatus = dbEditNone;
     return true;
}

bool CADORecordset::Find(LPCTSTR lpFind, int nSearchDirection)
{

    m_strFind = lpFind;
    m_nSearchDirection = nSearchDirection;

    ASSERT(!m_strFind.empty());

     if (m_nSearchDirection == searchForward)
    {
		m_pRecordset->Find(_bstr_t(m_strFind.c_str()), 0 , adSearchForward, "");
         if (!IsEof())
        {
            m_varBookFind = m_pRecordset->Bookmark;
             return true;
        }
    }
     else   if (m_nSearchDirection == searchBackward)
    {
        m_pRecordset->Find(_bstr_t(m_strFind.c_str()), 0 , adSearchBackward, "");
         if (!IsBof())
        {
            m_varBookFind = m_pRecordset->Bookmark;
             return true;
        }
    }
     else 
    {
        TRACE("Unknown parameter. %d", nSearchDirection);
        m_nSearchDirection = searchForward;
    }
     return false;
}

bool CADORecordset::FindFirst(LPCTSTR lpFind)
{
    m_pRecordset->MoveFirst();
     return Find(lpFind);
}

bool CADORecordset::FindNext()
{
     if (m_nSearchDirection == searchForward)
    {
        m_pRecordset->Find(_bstr_t(m_strFind.c_str()), 1 , adSearchForward, m_varBookFind);
         if (!IsEof())
        {
            m_varBookFind = m_pRecordset->Bookmark;
             return true;
        }
    }
     else 
    {
        m_pRecordset->Find(_bstr_t(m_strFind.c_str()), 1 , adSearchBackward, m_varBookFind);
         if (!IsBof())
        {
            m_varBookFind = m_pRecordset->Bookmark;
             return true;
        }
    }
     return false;
}

bool CADORecordset::PutFieldValue(LPCTSTR lpFieldName, _variant_t vtFld)
{
     if (m_nEditStatus == dbEditNone)
         return false;
    
     try 
    {
        m_pRecordset->Fields->GetItem(lpFieldName)->Value = vtFld; 
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;    
    }
}


bool CADORecordset::PutFieldValue(_variant_t vtIndex, _variant_t vtFld)
{
     if (m_nEditStatus == dbEditNone)
         return false;

     try 
    {
        m_pRecordset->Fields->GetItem(vtIndex)->Value = vtFld;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::Clone(CADORecordset  & pRs)
{
     try 
    {
        pRs.m_pRecordset = m_pRecordset->Clone(adLockUnspecified);
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::SetFilter(LPCTSTR strFilter)
{
    ASSERT(IsOpen());
    
     try 
    {
        m_pRecordset->PutFilter(strFilter);
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::SetSort(LPCTSTR strCriteria)
{
    ASSERT(IsOpen());
    
     try 
    {
        m_pRecordset->PutSort(strCriteria);
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::SaveAsXML(LPCTSTR lpstrXMLFile)
{
    HRESULT hr;

    ASSERT(IsOpen());
    
     try 
    {
        hr = m_pRecordset->Save(lpstrXMLFile, adPersistXML);
         return hr == S_OK;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
     return true;
}

bool CADORecordset::OpenXML(LPCTSTR lpstrXMLFile)
{
    HRESULT hr = S_OK;

     if (IsOpen())
        Close();

     try 
    {
        hr = m_pRecordset->Open(lpstrXMLFile, "Provider=MSPersist;", adOpenForwardOnly, adLockOptimistic, adCmdFile);
         return hr == S_OK;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADORecordset::Execute(CADOCommand *  pAdoCommand)
{
     if (IsOpen())
        Close();

    ASSERT(!pAdoCommand->GetText().empty());
     try 
    {
        m_pConnection->CursorLocation = adUseClient;
        m_pRecordset = pAdoCommand->GetCommand()->Execute(NULL, NULL, pAdoCommand->GetType());
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

void CADORecordset::dump_com_error(_com_error& e)
{
    std::wstring ErrorStr;
    
    
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
    base::SStringPrintf(&ErrorStr, L"CADORecordset Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n",
        e.Error(), e.ErrorMessage(), (LPCTSTR)bstrSource, (LPCTSTR)bstrDescription);
	base::SStringPrintf(&m_strLastError, L"Query = %s\r\n%s", GetQuery().c_str(), ErrorStr.c_str());
    m_dwLastError = e.Error();
    #ifdef _DEBUG
        MessageBox(NULL, ErrorStr.c_str(), L"Error", MB_OK | MB_ICONERROR);
    #endif      
}


////////////////////////////////////////////////////// / 
// 
//  CADOCommad Class
//

CADOCommand::CADOCommand(CADODatabase *  pAdoDatabase, std::wstring strCommandText, int nCommandType)
{
    m_pCommand = NULL;
    m_pCommand.CreateInstance(__uuidof(Command));
    m_strCommandText = strCommandText;
    //m_pCommand->CommandText = m_strCommandText.AllocSysString();
	m_pCommand->CommandText = ::ConvertStringToBSTR(m_strCommandText);
    m_nCommandType = nCommandType;
    m_pCommand->CommandType = (CommandTypeEnum)m_nCommandType;
    m_pCommand->ActiveConnection = pAdoDatabase->GetActiveConnection();    
    m_nRecordsAffected =  0 ;
}

bool CADOCommand::AddParameter(CADOParameter *  pAdoParameter)
{
    ASSERT(pAdoParameter->GetParameter() != NULL);

     try 
    {
        m_pCommand->Parameters->Append(pAdoParameter->GetParameter());
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADOCommand::AddParameter(std::wstring strName, int nType, int nDirection, long lSize, int nValue)
{

    _variant_t vtValue;

    vtValue.vt = VT_I2;
    vtValue.iVal = nValue;

     return AddParameter(strName, nType, nDirection, lSize, vtValue);
}

bool CADOCommand::AddParameter(std::wstring strName, int nType, int nDirection, long lSize, long lValue)
{

    _variant_t vtValue;

    vtValue.vt = VT_I4;
    vtValue.lVal = lValue;

     return AddParameter(strName, nType, nDirection, lSize, vtValue);
}

bool CADOCommand::AddParameter(std::wstring strName, int nType, int nDirection, long lSize, double  dblValue, int nPrecision, int nScale)
{

    _variant_t vtValue;

    vtValue.vt = VT_R8;
    vtValue.dblVal = dblValue;

     return AddParameter(strName, nType, nDirection, lSize, vtValue, nPrecision, nScale);
}

bool CADOCommand::AddParameter(std::wstring strName, int nType, int nDirection, long lSize, std::wstring strValue)
{
	_variant_t vtValue;

	vtValue.vt = VT_BSTR;
	//strValue.AllocSysString();
	vtValue.bstrVal = ::ConvertStringToBSTR(strValue);
	bool b_ret = AddParameter(strName, nType, nDirection, lSize, vtValue);
	//::SysFreeString(vtValue.bstrVal);
	return b_ret;
}

/*bool CADOCommand::AddParameter(std::wstring strName, int nType, int nDirection, long lSize, COleDateTime time)
{

    _variant_t vtValue;

    vtValue.vt = VT_DATE;
    vtValue.date = time;

     return AddParameter(strName, nType, nDirection, lSize, vtValue);
}*/


bool CADOCommand::AddParameter(std::wstring strName, int nType, int nDirection, long lSize, _variant_t vtValue, int nPrecision, int nScale)
{
	BSTR bstrName = ::ConvertStringToBSTR(strName);
	bool b_ret = false;

    try 
    {
        _ParameterPtr pParam = m_pCommand->CreateParameter(bstrName/*strName.AllocSysString()*/, (DataTypeEnum)nType, (ParameterDirectionEnum)nDirection, lSize, vtValue);
        pParam->PutPrecision(nPrecision);
        pParam->PutNumericScale(nScale);
        m_pCommand->Parameters->Append(pParam);
        
        b_ret = true;
    }
    catch (_com_error& e)
    {
        dump_com_error(e);
        b_ret = false;
    }

	//::SysFreeString(bstrName);
	return b_ret;
}


void CADOCommand::SetText(std::wstring strCommandText)
{
    ASSERT(!strCommandText.empty());

    m_strCommandText = strCommandText;
    m_pCommand->CommandText = ::ConvertStringToBSTR(m_strCommandText);//m_strCommandText.AllocSysString();
}

void CADOCommand::SetType(int nCommandType)
{
    m_nCommandType = nCommandType;
    m_pCommand->CommandType = (CommandTypeEnum)m_nCommandType;
}

bool CADOCommand::Execute()
{
    _variant_t vRecords;
    m_nRecordsAffected =  0 ;
     try 
    {
        m_pCommand->Execute(& vRecords, NULL, adCmdStoredProc);
        m_nRecordsAffected = vRecords.iVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

void CADOCommand::dump_com_error(_com_error& e)
{
    std::wstring ErrorStr;
    
    
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
    base::SStringPrintf(&ErrorStr, L"CADOCommand Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n",
        e.Error(), e.ErrorMessage(), (LPCTSTR)bstrSource, (LPCTSTR)bstrDescription);
    m_strLastError = ErrorStr;
    m_dwLastError = e.Error();
    #ifdef _DEBUG
        MessageBox(NULL, ErrorStr.c_str(), L"Error", MB_OK | MB_ICONERROR);
     #endif      
}


////////////////////////////////////////////////////// / 
// 
//  CADOParameter Class
//

CADOParameter::CADOParameter(int nType, long lSize, int nDirection, std::wstring strName)
{
    m_pParameter = NULL;
    m_pParameter.CreateInstance(__uuidof(Parameter));
    m_strName = _T("");
    m_pParameter->Direction = (ParameterDirectionEnum)nDirection;
    m_strName = strName;
    //m_pParameter->Name = m_strName.AllocSysString();
	m_pParameter->Name = ::ConvertStringToBSTR(m_strName);
    m_pParameter->Type = (DataTypeEnum)nType;
    m_pParameter->Size = lSize;
    m_nType = nType;
}

bool CADOParameter::SetValue(int nValue)
{
    _variant_t vtVal;

    ASSERT(m_pParameter != NULL);
    
    vtVal.vt = VT_I2;
    vtVal.iVal = nValue;

     try 
    {
         if (m_pParameter->Size ==  0)
            m_pParameter->Size =  sizeof (int);

        m_pParameter->Value = vtVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}


bool CADOParameter::SetValue(long lValue)
{
    _variant_t vtVal;

    ASSERT(m_pParameter != NULL);
    
    vtVal.vt = VT_I4;
    vtVal.lVal = lValue;

     try 
    {
         if (m_pParameter->Size ==  0)
            m_pParameter->Size =  sizeof (long);

        m_pParameter->Value = vtVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADOParameter::SetValue(double  dblValue)
{
    _variant_t vtVal;

    ASSERT(m_pParameter != NULL);
    
    vtVal.vt = VT_R8;
    vtVal.dblVal = dblValue;

     try 
    {
         if (m_pParameter->Size ==  0)
            m_pParameter->Size =  sizeof (double);

        m_pParameter->Value = vtVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADOParameter::SetValue(std::wstring strValue)
{
    _variant_t vtVal;

    ASSERT(m_pParameter != NULL);
    
     if (!strValue.empty())
        vtVal.vt = VT_BSTR;
     else 
        vtVal.vt = VT_NULL;

    // Corrected by Giles Forster 10/03/2001 
    //strValue.AllocSysString();
	vtVal.bstrVal = ::ConvertStringToBSTR(strValue);
	bool b_ret = false;
    try 
    {
        if (m_pParameter->Size ==  0)
            m_pParameter->Size = sizeof(char) * strValue.length();

        m_pParameter->Value = vtVal;
        b_ret = true;
    }
    catch (_com_error& e)
    {
        dump_com_error(e);
        b_ret = false;
    }

	//::SysFreeString(vtVal.bstrVal);
	return b_ret;
}

/*bool CADOParameter::SetValue(COleDateTime time)
{
    _variant_t vtVal;

    ASSERT(m_pParameter != NULL);
    
    vtVal.vt = VT_DATE;
    vtVal.date = time;

     try 
    {
         if (m_pParameter->Size ==  0)
            m_pParameter->Size =  sizeof (DATE);

        m_pParameter->Value = vtVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}*/

bool CADOParameter::SetValue(_variant_t vtValue)
{

    ASSERT(m_pParameter != NULL);

     try 
    {
         if (m_pParameter->Size ==  0)
            m_pParameter->Size =  sizeof (VARIANT);
        
        m_pParameter->Value = vtValue;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADOParameter::GetValue(int& nValue)
{
    _variant_t vtVal;
     int nVal =  0 ;

     try 
    {
        vtVal = m_pParameter->Value;

         switch (vtVal.vt)
        {
         case VT_BOOL:
            nVal = vtVal.boolVal;
             break;
         case VT_I2:
         case VT_UI1:
            nVal = vtVal.iVal;
             break;
         case VT_INT:
            nVal = vtVal.intVal;
             break;
         case VT_NULL:
         case VT_EMPTY:
            nVal =  0 ;
             break;
         default:
            nVal = vtVal.iVal;
        }    
        nValue = nVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADOParameter::GetValue(long &  lValue)
{
    _variant_t vtVal;
     long lVal =  0 ;

     try 
    {
        vtVal = m_pParameter->Value;
         if (vtVal.vt != VT_NULL  &&  vtVal.vt != VT_EMPTY)
            lVal = vtVal.lVal;
        lValue = lVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADOParameter::GetValue(double &  dbValue)
{
    _variant_t vtVal;
     double  dblVal;
     try 
    {
        vtVal = m_pParameter->Value;
         switch (vtVal.vt)
        {
         case VT_R4:
            dblVal = vtVal.fltVal;
             break;
         case VT_R8:
            dblVal = vtVal.dblVal;
             break;
         case VT_DECIMAL:
             // Corrected by Jos?Carlos Martínez Galán 
            dblVal = vtVal.decVal.Lo32;
            dblVal  *=  (vtVal.decVal.sign ==  128) ?   - 1  :  1 ;
            dblVal  /=  pow((double)10 , vtVal.decVal.scale); 
             break;
         case VT_UI1:
            dblVal = vtVal.iVal;
             break;
         case VT_I2:
         case VT_I4:
            dblVal = vtVal.lVal;
             break;
         case VT_INT:
            dblVal = vtVal.intVal;
             break;
         case VT_NULL:
         case VT_EMPTY:
            dblVal =  0 ;
             break;
         default:
            dblVal =  0 ;
        }
        dbValue = dblVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

bool CADOParameter::GetValue(std::wstring& strValue, std::wstring strDateFormat)
{
    _variant_t vtVal;
    std::wstring strVal = _T("");

     try 
    {
        vtVal = m_pParameter->Value;
         switch (vtVal.vt) 
        {
         case VT_R4:
            strVal = DblToStr(vtVal.fltVal);
             break;
         case VT_R8:
            strVal = DblToStr(vtVal.dblVal);
             break;
         case VT_BSTR:
            strVal = vtVal.bstrVal;
             break;
         case VT_I2:
         case VT_UI1:
            strVal = IntToStr(vtVal.iVal);
             break;
         case VT_INT:
            strVal = IntToStr(vtVal.intVal);
             break;
         case VT_I4:
            strVal = LongToStr(vtVal.lVal);
             break;
         case VT_DECIMAL:
            {
             // Corrected by Jos?Carlos Martínez Galán 
            double  val = vtVal.decVal.Lo32;
            val  *=  (vtVal.decVal.sign ==  128) ?   - 1  :  1 ;
            val  /=  pow((double)10 , vtVal.decVal.scale); 
            strVal = DblToStr(val);
            }
             break;
         case VT_DATE:
            {
                /*COleDateTime dt(vtVal);

                if (strDateFormat.empty())
                    strDateFormat = _T("%Y-%m-%d %H:%M:%S");
                strVal = dt.Format(strDateFormat);*/
				//Date stamp
				strVal = base::UintToString16(vtVal);
            }
            break;
         case VT_EMPTY:
         case VT_NULL:
            strVal.clear();
             break;
         default:
            strVal.clear();
             return false;
        }
        strValue = strVal;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}

/*bool CADOParameter::GetValue(COleDateTime &  time)
{
    _variant_t vtVal;

     try 
    {
        vtVal = m_pParameter->Value;
         switch (vtVal.vt) 
        {
         case VT_DATE:
            {
                COleDateTime dt(vtVal);
                time = dt;
            }
             break;
         case VT_EMPTY:
         case VT_NULL:
            time.SetStatus(COleDateTime:: null);
             break;
         default:
             return false;
        }
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}*/

bool CADOParameter::GetValue(_variant_t &  vtValue)
{
     try 
    {
        vtValue = m_pParameter->Value;
         return true;
    }
     catch (_com_error& e)
    {
        dump_com_error(e);
         return false;
    }
}


void CADOParameter::dump_com_error(_com_error& e)
{
    std::wstring ErrorStr;
    
    
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());
    base::SStringPrintf(&ErrorStr, L"CADOParameter Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n",
        e.Error(), e.ErrorMessage(), (LPCTSTR)bstrSource, (LPCTSTR)bstrDescription);

    m_strLastError = ErrorStr;
    m_dwLastError = e.Error();
    #ifdef _DEBUG
        MessageBox(NULL, ErrorStr.c_str(), L"Error", MB_OK | MB_ICONERROR);
    #endif      
}

//IMPLEMENT_DYNAMIC(CADOException, CException)

CADOException::CADOException(int nCause, std::wstring strErrorString)/* : CException(true)*/
{
    m_nCause = nCause;
    m_strErrorString = strErrorString;
}

CADOException::~CADOException()
{

}

int CADOException::GetError(int nADOError)
{
     switch  (nADOError)
    {
     case noError:
         return CADOException::noError;
         break;
     default:
         return CADOException::Unknown;
    }
    
}

void AfxThrowADOException(int nADOError, std::wstring strErrorString)
{
     throw new CADOException(nADOError, strErrorString);
}

//»Ö¸´
#pragma warning(pop)