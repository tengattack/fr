

#ifndef _TA_SCRIPT_SQLITE_
#define _TA_SCRIPT_SQLITE_

#pragma once

#include <base/basictypes.h>
#include <base/lock.h>

#include <string>
#include <vector>
#include <map>

#include <sqlite3.h>

enum ColType {
	kCTUnknow = 0,
	kCTInteger,
	kCTFloat,
	kCTString,
	kCTBLOB,
	kCTNULL
};

typedef struct _sqlite_row_data {
	union {
		char *string_;
		unsigned char *blob_;
		int32 int32_;
		uint32 uint32_;
		int64 int64_;
		uint64 uint64_;
		double double_;
	};
	int byteslen;
	ColType type;
} sqlite_row_data;

typedef struct _sqlite_col {
	char *name;
} sqlite_col;


class SqliteQueryResult {
public:
	SqliteQueryResult();
	~SqliteQueryResult();

	//int add_row(LPCSTR col, void *data, int irow = -1);
	int add_row(int icol, sqlite_row_data *row_data, int irow = -1);
	int add_col(LPCSTR col);

	inline int GetColCount() {
		return m_col_count;
	}

	inline int GetRowCount() {
		return m_rows.size();
	}

	inline bool GetRowData(int i, int icol, sqlite_row_data *row) {
		if (i >= 0 && i < m_rows.size() && 
			icol >= 0 && icol < m_col_count) {

			*row = m_rows[i][icol];
			return true;
		} else {
			return false;
		}
	}

	inline LPCSTR GetColName(int i) {
		if (i >= 0 && i < m_cols.size()) {
			return m_cols[i].name;
		}
		return NULL;
	}

	void clear();

protected:
	std::vector<sqlite_col> m_cols;
	std::vector<sqlite_row_data *> m_rows;

	int m_col_count;
};

class CSqlite {
public:
	CSqlite();
	~CSqlite();

	bool Open(LPCWSTR path);
	inline bool IsOpen() {
		return m_sqlite_ != NULL;
	}

	void Close();

	virtual bool BeginTrans();
	virtual bool Commit();
	virtual bool Rollback();

	virtual bool Query(const char* query);

	inline LPCWSTR GetErrorMessage() {
		return m_error_msg_.c_str();
	}

	inline int GetErrorCode() {
		return m_error_code_;
	}

	inline SqliteQueryResult& GetResult() {
		return m_result;
	}

	inline int64 GetLastInsertId() {
		return sqlite3_last_insert_rowid(m_sqlite_);
	}

	static void Init();

protected:
	std::wstring m_path_;
	std::wstring m_error_msg_;
	int m_error_code_;
	sqlite3 *m_sqlite_;

	SqliteQueryResult m_result;
	bool m_set_encoding;
	Lock m_trans_lock;

	void GetError();

	static int _callback_exec(void *notused, int argc, char **argv, char **aszColName);
};

#endif //_TA_SCRIPT_SQLITE_