// 
//  MODULE: Ado2.h
// 
//   AUTHOR: Carlos Antollini 
// 
//  mailto: cantollini@hotmail.com
// 
//   Date: 07/02/2003
// 
//   Version 2.10
//  

#if !defined(_TA_CUTE_ADO2_H_INCLUDED_) 
#define _TA_CUTE_ADO2_H_INCLUDED_ 

#if _MSC_VER >= 1000 
#pragma once
#endif  // _MSC_VER >= 1000

#include <math.h> 
//#include "C:\Program Files\Microsoft Analysis Services\Samples\Include\msmd.h" 

#pragma warning(push)
#pragma warning(disable: 4146)
#pragma warning(disable: 4244)	//close x64 long to x86 long warning

// CG: In order to use this code against a different version of ADO, the appropriate
// ADO library needs to be used in the #import statement 
#import "D:\Code\msadox.dll"
#import "D:\Code\msado15.dll" rename("EOF", "EndOfFile")

#ifdef C_JET_ENGINE_IMPORT
#import "c:\Program Files\Common Files\System\ado\MSJRO.DLL" no_namespace
#endif

using namespace ADODB;

#include "icrsint.h"

class CADOCommand;

struct CADOFieldInfo
{
	std::wstring m_strName; 
	short m_nType;
	long m_lSize; 
	long m_lDefinedSize;
	long m_lAttributes;
	short m_nOrdinalPosition;
	bool m_bRequired;  
	bool m_bAllowZeroLength; 
	long m_lCollatingOrder; 
};

std::wstring IntToStr(int nVal);

std::wstring LongToStr(long lVal);

std::wstring ULongToStr(unsigned long ulVal);

std::wstring DblToStr(double dblVal, int ndigits = 20);

std::wstring DblToStr(float fltVal);

#ifdef C_JET_ENGINE_IMPORT
class CJetEngine
{
public:

  CJetEngine()
  {
   ;
  }

   virtual  ~ CJetEngine()
  {
   ;
  }

   static bool CCompactDatabase(std::wstring strDatabaseSource, std::wstring strDatabaseDestination);
};
#endif

class CADODatabase
{
public:
	enum cadoConnectModeEnum
	{  
		connectModeUnknown = adModeUnknown,
		connectModeRead = adModeRead,
		connectModeWrite = adModeWrite,
		connectModeReadWrite = adModeReadWrite,
		connectModeShareDenyRead = adModeShareDenyRead,
		connectModeShareDenyWrite = adModeShareDenyWrite,
		connectModeShareExclusive = adModeShareExclusive,
		connectModeShareDenyNone = adModeShareDenyNone
	};

	CADODatabase()
	{
		::CoInitialize(NULL);
      
		m_pConnection = NULL;
		m_strConnection = _T("");
		m_strLastError = _T("");
		m_dwLastError = 0;
		m_pConnection.CreateInstance(__uuidof(Connection));
		m_nRecordsAffected = 0;
		m_nConnectionTimeout = 0;
	}
  
	virtual  ~CADODatabase()
	{
		Close();
		m_pConnection.Release();
		m_pConnection = NULL;
		m_strConnection = _T("");
		m_strLastError = _T("");
		m_dwLastError = 0;
		::CoUninitialize();
	}

  bool Create(LPCTSTR lpstrFileName);
  bool Open(LPCTSTR lpstrConnection = _T(""), LPCTSTR lpstrUserID = _T(""), LPCTSTR lpstrPassword = _T(""));
  _ConnectionPtr GetActiveConnection() 
    { return m_pConnection;};
  bool Execute(LPCTSTR lpstrExec);
   int GetRecordsAffected()
    { return m_nRecordsAffected;};
  DWORD GetRecordCount(_RecordsetPtr m_pRs);
   long BeginTransaction() 
    { return m_pConnection -> BeginTrans();};
   long CommitTransaction() 
    { return m_pConnection -> CommitTrans();};
   long RollbackTransaction() 
    { return m_pConnection -> RollbackTrans();};
  bool IsOpen();
   void Close();
   void SetConnectionMode(cadoConnectModeEnum nMode)
    {m_pConnection -> PutMode((enum ConnectModeEnum)nMode);};
   void SetConnectionString(LPCTSTR lpstrConnection)
    {m_strConnection = lpstrConnection;};
  std::wstring GetConnectionString()
    { return m_strConnection;};
  std::wstring GetLastErrorString() 
    { return m_strLastError;};
  DWORD GetLastError()
    { return m_dwLastError;};
  std::wstring GetErrorDescription() 
    { return m_strErrorDescription;};
   void SetConnectionTimeout(long nConnectionTimeout = 30)
    {m_nConnectionTimeout = nConnectionTimeout;};

protected:
	void dump_com_error(_com_error & e);

public:
	_ConnectionPtr m_pConnection;

protected:
	std::wstring m_strConnection;
	std::wstring m_strLastError;
	std::wstring m_strErrorDescription;
	DWORD m_dwLastError;
	int m_nRecordsAffected;
	long m_nConnectionTimeout;
};

class CADORecordset
{
public:
  bool Clone(CADORecordset & pRs);
  
   enum cadoOpenEnum
  {
    openUnknown = 0,
    openQuery = 1,
    openTable = 2,
    openStoredProc = 3 
  };

   enum cadoEditEnum
  {
    dbEditNone = 0,
    dbEditNew = 1,
    dbEdit = 2 
  };
  
   enum cadoPositionEnum
  {
  
    positionUnknown = - 1,
    positionBOF = - 2,
    positionEOF = - 3 
  };
  
   enum cadoSearchEnum
  {  
    searchForward = 1,
    searchBackward = - 1 
  };

   enum cadoDataType
  {
    typeEmpty = ADODB::adEmpty,
    typeTinyInt = ADODB::adTinyInt,
    typeSmallInt = ADODB::adSmallInt,
    typeInteger = ADODB::adInteger,
    typeBigInt = ADODB::adBigInt,
    typeUnsignedTinyInt = ADODB::adUnsignedTinyInt,
    typeUnsignedSmallInt = ADODB::adUnsignedSmallInt,
    typeUnsignedInt = ADODB::adUnsignedInt,
    typeUnsignedBigInt = ADODB::adUnsignedBigInt,
    typeSingle = ADODB::adSingle,
    typeDouble = ADODB::adDouble,
    typeCurrency = ADODB::adCurrency,
    typeDecimal = ADODB::adDecimal,
    typeNumeric = ADODB::adNumeric,
    typeboolean = ADODB::adBoolean,
    typeError = ADODB::adError,
    typeUserDefined = ADODB::adUserDefined,
    typeVariant = ADODB::adVariant,
    typeIDispatch = ADODB::adIDispatch,
    typeIUnknown = ADODB::adIUnknown,
    typeGUID = ADODB::adGUID,
    typeDate = ADODB::adDate,
    typeDBDate = ADODB::adDBDate,
    typeDBTime = ADODB::adDBTime,
    typeDBTimeStamp = ADODB::adDBTimeStamp,
    typeBSTR = ADODB::adBSTR,
    typeChar = ADODB::adChar,
    typeVarChar = ADODB::adVarChar,
    typeLongVarChar = ADODB::adLongVarChar,
    typeWChar = ADODB::adWChar,
    typeVarWChar = ADODB::adVarWChar,
    typeLongVarWChar = ADODB::adLongVarWChar,
    typeBinary = ADODB::adBinary,
    typeVarBinary = ADODB::adVarBinary,
    typeLongVarBinary = ADODB::adLongVarBinary,
    typeChapter = ADODB::adChapter,
    typeFileTime = ADODB::adFileTime,
    typePropVariant = ADODB::adPropVariant,
    typeVarNumeric = ADODB::adVarNumeric,
    typeArray = ADODB::adVariant
  };
  
   enum cadoSchemaType 
  {
    schemaSpecific = adSchemaProviderSpecific,  
    schemaAsserts = adSchemaAsserts,
    schemaCatalog = adSchemaCatalogs,
    schemaCharacterSet = adSchemaCharacterSets,
    schemaCollections = adSchemaCollations,
    schemaColumns = adSchemaColumns,
    schemaConstraints = adSchemaCheckConstraints,
    schemaConstraintColumnUsage = adSchemaConstraintColumnUsage,
    schemaConstraintTableUsage  = adSchemaConstraintTableUsage,
    shemaKeyColumnUsage = adSchemaKeyColumnUsage,
    schemaTableConstraints = adSchemaTableConstraints,
    schemaColumnsDomainUsage = adSchemaColumnsDomainUsage,
    schemaIndexes = adSchemaIndexes,
    schemaColumnPrivileges = adSchemaColumnPrivileges,
    schemaTablePrivileges = adSchemaTablePrivileges,
    schemaUsagePrivileges = adSchemaUsagePrivileges,
    schemaProcedures = adSchemaProcedures,
    schemaTables = adSchemaTables,
    schemaProviderTypes = adSchemaProviderTypes,
    schemaViews = adSchemaViews,
    schemaProcedureParameters = adSchemaProcedureParameters,
    schemaForeignKeys = adSchemaForeignKeys,
    schemaPrimaryKeys = adSchemaPrimaryKeys,
    schemaProcedureColumns = adSchemaProcedureColumns,
    schemaDBInfoKeywords = adSchemaDBInfoKeywords,
    schemaDBInfoLiterals = adSchemaDBInfoLiterals,
    schemaCubes = adSchemaCubes,
    schemaDimensions = adSchemaDimensions,
    schemaHierarchies  = adSchemaHierarchies, 
    schemaLevels = adSchemaLevels,
    schemaMeasures = adSchemaMeasures,
    schemaProperties = adSchemaProperties,
    schemaMembers = adSchemaMembers,
  }; 


  bool SetFieldValue(int nIndex, int nValue);
  bool SetFieldValue(LPCTSTR lpFieldName, int nValue);
  bool SetFieldValue(int nIndex, long lValue);
  bool SetFieldValue(LPCTSTR lpFieldName, long lValue);
  bool SetFieldValue(int nIndex, unsigned long lValue);
  bool SetFieldValue(LPCTSTR lpFieldName, unsigned long lValue);
  bool SetFieldValue(int nIndex, double dblValue);
  bool SetFieldValue(LPCTSTR lpFieldName, double dblValue);
  bool SetFieldValue(int nIndex, std::wstring strValue);
  bool SetFieldValue(LPCTSTR lpFieldName, std::wstring strValue);
//  bool SetFieldValue(int nIndex, COleDateTime time);
//  bool SetFieldValue(LPCTSTR lpFieldName, COleDateTime time);
  bool SetFieldValue(int nIndex, bool bValue);
  bool SetFieldValue(LPCTSTR lpFieldName, bool bValue);
//  bool SetFieldValue(int nIndex, COleCurrency cyValue);
//  bool SetFieldValue(LPCTSTR lpFieldName, COleCurrency cyValue);
  bool SetFieldValue(int nIndex, _variant_t vtValue);
  bool SetFieldValue(LPCTSTR lpFieldName, _variant_t vtValue);

  bool SetFieldEmpty(int nIndex);
  bool SetFieldEmpty(LPCTSTR lpFieldName);

   void CancelUpdate();
  bool Update();
   void Edit();
  bool AddNew();
  bool AddNew(CADORecordBinding & pAdoRecordBinding);

  bool Find(LPCTSTR lpFind, int nSearchDirection = CADORecordset::searchForward);
  bool FindFirst(LPCTSTR lpFind);
  bool FindNext();

  CADORecordset();

  CADORecordset(CADODatabase * pAdoDatabase);

   virtual  ~ CADORecordset()
  {
    Close();
     if (m_pRecordset)
      m_pRecordset.Release();
     if (m_pCmd)
      m_pCmd.Release();
    m_pRecordset = NULL;
    m_pCmd = NULL;
    m_pRecBinding = NULL;
    m_strQuery = _T("");
    m_strLastError = _T("");
    m_dwLastError = 0;
    m_nEditStatus = dbEditNone;
  }

  std::wstring GetQuery() 
    { return m_strQuery;};
   void SetQuery(LPCTSTR strQuery) 
    {m_strQuery = strQuery;};
  bool RecordBinding(CADORecordBinding & pAdoRecordBinding);
  DWORD GetRecordCount();
  bool IsOpen();
   void Close();
  bool Open(_ConnectionPtr mpdb, LPCTSTR lpstrExec = _T(""), int nOption = CADORecordset::openUnknown);
  bool Open(LPCTSTR lpstrExec = _T(""), int nOption = CADORecordset::openUnknown);
  bool OpenSchema(int nSchema, LPCTSTR SchemaID = _T(""));
   long GetFieldCount()
    { return m_pRecordset -> Fields -> GetCount();};
  bool GetFieldValue(LPCTSTR lpFieldName, int & nValue);
  bool GetFieldValue(int nIndex, int & nValue);
  bool GetFieldValue(LPCTSTR lpFieldName, long & lValue);
  bool GetFieldValue(int nIndex, long & lValue);
  bool GetFieldValue(LPCTSTR lpFieldName, unsigned long & ulValue);
  bool GetFieldValue(int nIndex, unsigned long & ulValue);
  bool GetFieldValue(LPCTSTR lpFieldName, double & dbValue);
  bool GetFieldValue(int nIndex, double & dbValue);
  bool GetFieldValue(LPCTSTR lpFieldName, std::wstring & strValue, std::wstring strDateFormat = _T(""));
  bool GetFieldValue(int nIndex, std::wstring & strValue, std::wstring strDateFormat = _T(""));
//  bool GetFieldValue(LPCTSTR lpFieldName, COleDateTime & time);
//  bool GetFieldValue(int nIndex, COleDateTime & time);
  bool GetFieldValue(int nIndex, bool & bValue);
  bool GetFieldValue(LPCTSTR lpFieldName, bool & bValue);
//  bool GetFieldValue(int nIndex, COleCurrency & cyValue);
//  bool GetFieldValue(LPCTSTR lpFieldName, COleCurrency & cyValue);
  bool GetFieldValue(int nIndex, _variant_t & vtValue);
  bool GetFieldValue(LPCTSTR lpFieldName, _variant_t & vtValue);
  
  bool IsFieldNull(LPCTSTR lpFieldName);
  bool IsFieldNull(int nIndex);
  bool IsFieldEmpty(LPCTSTR lpFieldName);
  bool IsFieldEmpty(int nIndex);  
  bool IsEof()
    { return m_pRecordset -> EndOfFile == VARIANT_TRUE;};
  bool IsEOF()
    { return m_pRecordset -> EndOfFile == VARIANT_TRUE;};
  bool IsBof()
    { return m_pRecordset -> BOF == VARIANT_TRUE;};
  bool IsBOF()
    { return m_pRecordset -> BOF == VARIANT_TRUE;};
   void MoveFirst() 
    {m_pRecordset -> MoveFirst();};
   void MoveNext() 
    {m_pRecordset -> MoveNext();};
   void MovePrevious() 
    {m_pRecordset -> MovePrevious();};
   void MoveLast() 
    {m_pRecordset -> MoveLast();};
   long GetAbsolutePage()
    { return m_pRecordset -> GetAbsolutePage();};
   void SetAbsolutePage(int nPage)
    {m_pRecordset -> PutAbsolutePage((enum PositionEnum)nPage);};
   long GetPageCount()
    { return m_pRecordset -> GetPageCount();};
   long GetPageSize()
    { return m_pRecordset -> GetPageSize();};
   void SetPageSize(int nSize)
    {m_pRecordset -> PutPageSize(nSize);};
   long GetAbsolutePosition()
    { return m_pRecordset -> GetAbsolutePosition();};
   void SetAbsolutePosition(int nPosition)
    {m_pRecordset -> PutAbsolutePosition((enum PositionEnum)nPosition);};
  bool GetFieldInfo(LPCTSTR lpFieldName, CADOFieldInfo * fldInfo);
  bool GetFieldInfo(int nIndex, CADOFieldInfo * fldInfo);
  bool AppendChunk(LPCTSTR lpFieldName, LPVOID lpData, UINT nBytes);
  bool AppendChunk(int nIndex, LPVOID lpData, UINT nBytes);

  bool GetChunk(LPCTSTR lpFieldName, std::wstring & strValue);
  bool GetChunk(int nIndex, std::wstring & strValue);
  
  bool GetChunk(LPCTSTR lpFieldName, LPVOID pData);
  bool GetChunk(int nIndex, LPVOID pData);

  std::wstring GetString(LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows = 0);
  std::wstring GetLastErrorString() 
    { return m_strLastError;};
  DWORD GetLastError()
    { return m_dwLastError;};
   void GetBookmark()
    {m_varBookmark = m_pRecordset -> Bookmark;};
  bool SetBookmark();
  bool Delete();
  bool IsConnectionOpen()
    { return m_pConnection != NULL && m_pConnection -> GetState() != adStateClosed;};
  _RecordsetPtr GetRecordset()
    { return m_pRecordset;};
  _ConnectionPtr GetActiveConnection() 
    { return m_pConnection;};

  bool SetFilter(LPCTSTR strFilter);
  bool SetSort(LPCTSTR lpstrCriteria);
  bool SaveAsXML(LPCTSTR lpstrXMLFile);
  bool OpenXML(LPCTSTR lpstrXMLFile);
  bool Execute(CADOCommand * pCommand);
  bool Requery();

public:
  _RecordsetPtr m_pRecordset;
  _CommandPtr m_pCmd;
  
protected:
  _ConnectionPtr m_pConnection;
   int m_nSearchDirection;
  std::wstring m_strFind;
  _variant_t m_varBookFind;
  _variant_t m_varBookmark;
   int m_nEditStatus;
  std::wstring m_strLastError;
  DWORD m_dwLastError;
   void dump_com_error(_com_error & e);
  IADORecordBinding * m_pRecBinding;
  std::wstring m_strQuery;

protected:
  bool PutFieldValue(LPCTSTR lpFieldName, _variant_t vtFld);
  bool PutFieldValue(_variant_t vtIndex, _variant_t vtFld);
  bool GetFieldInfo(FieldPtr pField, CADOFieldInfo * fldInfo);
  bool GetChunk(FieldPtr pField, std::wstring & strValue);
  bool GetChunk(FieldPtr pField, LPVOID lpData);
  bool AppendChunk(FieldPtr pField, LPVOID lpData, UINT nBytes);
    
};

class CADOParameter
{
public:

   enum cadoParameterDirection
  {
    paramUnknown = adParamUnknown,
    paramInput = adParamInput,
    paramOutput = adParamOutput,
    paramInputOutput = adParamInputOutput,
    paramReturnValue = adParamReturnValue 
  };

  CADOParameter(int nType, long lSize = 0, int nDirection = paramInput, std::wstring strName = _T(""));
  
   virtual  ~ CADOParameter()
  {
    m_pParameter.Release();
    m_pParameter = NULL;
    m_strName = _T("");
  }

  bool SetValue(int nValue);
  bool SetValue(long lValue);
  bool SetValue(double dbValue);
  bool SetValue(std::wstring strValue);
//  bool SetValue(COleDateTime time);
  bool SetValue(_variant_t vtValue);
  bool GetValue(int & nValue);
  bool GetValue(long & lValue);
  bool GetValue(double & dbValue);
  bool GetValue(std::wstring & strValue, std::wstring strDateFormat = _T(""));
//  bool GetValue(COleDateTime & time);
  bool GetValue(_variant_t & vtValue);
   void SetPrecision(int nPrecision)
    {m_pParameter -> PutPrecision(nPrecision);};
   void SetScale(int nScale)
    {m_pParameter -> PutNumericScale(nScale);};

   void SetName(std::wstring strName)
    {m_strName = strName;};
  std::wstring GetName()
    { return m_strName;};
   int GetType()
    { return m_nType;};
  _ParameterPtr GetParameter()
    { return m_pParameter;};

protected:
   void dump_com_error(_com_error & e);
  
protected:
  _ParameterPtr m_pParameter;
  std::wstring m_strName;
   int m_nType;
  std::wstring m_strLastError;
  DWORD m_dwLastError;
};

class CADOCommand
{
public:
   enum cadoCommandType
  {
    typeCmdText = adCmdText,
    typeCmdTable = adCmdTable,
    typeCmdTableDirect = adCmdTableDirect,
    typeCmdStoredProc = adCmdStoredProc,
    typeCmdUnknown = adCmdUnknown,
    typeCmdFile = adCmdFile
  };
  
  CADOCommand(CADODatabase * pAdoDatabase, std::wstring strCommandText = _T(""), int nCommandType = typeCmdStoredProc);
    
   virtual  ~ CADOCommand()
  {
    m_pCommand.Release();
    m_pCommand = NULL;
    m_strCommandText = _T("");
  }

   void SetTimeout(long nTimeOut)
    {m_pCommand -> PutCommandTimeout(nTimeOut);};
   void SetText(std::wstring strCommandText);
   void SetType(int nCommandType);
   int GetType()
    { return m_nCommandType;};
  bool AddParameter(CADOParameter * pAdoParameter);
  bool AddParameter(std::wstring strName, int nType, int nDirection, long lSize, int nValue);
  bool AddParameter(std::wstring strName, int nType, int nDirection, long lSize, long lValue);
  bool AddParameter(std::wstring strName, int nType, int nDirection, long lSize, double dblValue, int nPrecision = 0, int nScale = 0);
  bool AddParameter(std::wstring strName, int nType, int nDirection, long lSize, std::wstring strValue);
//  bool AddParameter(std::wstring strName, int nType, int nDirection, long lSize, COleDateTime time);
  bool AddParameter(std::wstring strName, int nType, int nDirection, long lSize, _variant_t vtValue, int nPrecision = 0, int nScale = 0);
  std::wstring GetText()
    { return m_strCommandText;};
  bool Execute();
   int GetRecordsAffected()
    { return m_nRecordsAffected;};
  _CommandPtr GetCommand()
    { return m_pCommand;};

protected:
   void dump_com_error(_com_error & e);

protected:
  _CommandPtr m_pCommand;
   int m_nCommandType;
   int m_nRecordsAffected;
  std::wstring m_strCommandText;
  std::wstring m_strLastError;
  DWORD m_dwLastError;
};

class CADOException/*: public CException*/
{
public:

   enum 
  {
    noError,   // no error 
    Unknown,   // unknown error 
  };

  //DECLARE_DYNAMIC(CADOException);
  
  CADOException(int nCause = 0, std::wstring strErrorString = _T(""));
   virtual  ~ CADOException();

   static  int GetError(int nADOError);

public:
   int m_nCause;
   std::wstring m_strErrorString;
  
protected:
  
};

void AfxThrowADOException(int nADOError = 1000, std::wstring strErrorString = _T(""));
BSTR ConvertStringToBSTR(std::wstring& str);

//»Ö¸´
#pragma warning(pop)

#endif  // AFX_ADO2_H_INCLUDED_ 