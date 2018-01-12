
#include "stdafx.h"
#include "sqlite.h"

#include <windows.h>

#include <sqlite3.h>

#include <common/strconv.h>

#pragma comment(lib, "sqlite3.lib")

SqliteQueryResult::SqliteQueryResult()
	: m_col_count(0)
{
}

SqliteQueryResult::~SqliteQueryResult()
{
	clear();
}

int SqliteQueryResult::add_row(int icol, sqlite_row_data *row_data, int irow)
{
	sqlite_row_data *row = NULL;
	if (irow == -1) {
		row = (sqlite_row_data *)malloc(sizeof(sqlite_row_data) * m_col_count);
		memset(row, 0, sizeof(sqlite_row_data) * m_col_count);

		if (row == NULL) {
			return -1;
		}

		m_rows.push_back(row);
		irow = m_rows.size() - 1;
	} else {
		row = m_rows[irow];
	}

	row[icol] = *row_data;
	return irow;
}

/*
int SqliteQueryResult::add_row(LPCSTR col, void *data, int type, int irow)
{
	int icol = add_col(col);

	sqlite_row_data *row = NULL;
	if (irow == -1) {
		row = (sqlite_row_data *)malloc(sizeof(sqlite_row_data) * m_col_count);
		memset(row, 0, sizeof(sqlite_row_data) * m_col_count);

		if (row == NULL) {
			return -1;
		}

		m_rows.push_back(row);
		irow = m_rows.size() - 1;
	} else {
		row = m_rows[irow];
	}

	/*switch (m_cols[icol].type) {
	case kCTString:
		row[icol].string_ = _strdup((const char *)data);
		break;
	case kCTSignedInteger:
		row[icol].uint32_ = *(uint32 *)data;
		break;
	case kCTUnsignedInteger:
		row[icol].int32_ = *(int32 *)data;
		break;
	default:
		//memcpy(&row[icol], data, sizeof(sqlite_row_data));
		break;
	}*

	return irow;
}*/

int SqliteQueryResult::add_col(LPCSTR col)
{
	int i = 0, j;
	for (i = 0; i < m_cols.size(); i++) {
		if (lstrcmpA(m_cols[i].name, col) == 0) {
			return i;
		}
	}

	sqlite_col sqlcol;
	//sqlcol.type = m_coltype_map[col].GetType();
	sqlcol.name = _strdup(col);

	m_cols.push_back(sqlcol);
	m_col_count++;

	sqlite_row_data *row, *new_row;
	for (j = 0; j < m_rows.size(); j++) {
		row = m_rows[j];
		new_row = (sqlite_row_data *)malloc(sizeof(sqlite_row_data) * m_col_count);
		memset(new_row, 0, sizeof(sqlite_row_data) * m_col_count);
		if (row) {
			memcpy(new_row, row, sizeof(sqlite_row_data) * (m_col_count - 1));
			free(row);
		}
		m_rows[j] = new_row;
	}

	return i;
}

void SqliteQueryResult::clear()
{
	int i, j;

	for (i = 0; i < m_rows.size(); i++) {
		for (j = 0; j < m_col_count; j++) {
			if (m_rows[i][j].type == kCTString || m_rows[i][j].type == kCTBLOB) {
				free(m_rows[i][j].string_);
			}
		}
		free(m_rows[i]);
	}
	m_rows.clear();

	for (j = 0; j < m_col_count; j++) {
		free(m_cols[j].name);
	}
	m_cols.clear();
	

	m_col_count = 0;
}

CSqlite::CSqlite()
	: m_sqlite_(NULL)
	, m_error_code_(SQLITE_OK)
	, m_set_encoding(false)
{
}

CSqlite::~CSqlite()
{
	Close();
}

void CSqlite::Init()
{
	if (sqlite3_threadsafe() == 0) {
		sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
	}
}

void CSqlite::GetError()
{
	const char *err_msg = NULL;
	m_error_code_=  sqlite3_errcode(m_sqlite_);
	err_msg = sqlite3_errmsg(m_sqlite_);
	wchar_t *wmsg = NULL;
	if (lo_C2W(&wmsg, err_msg)) {
		m_error_msg_ = wmsg;
		free(wmsg);
	}
}

bool CSqlite::Open(LPCWSTR path)
{
	if (!path) return false;

	Close();

	if (sqlite3_open16(path, &m_sqlite_) != SQLITE_OK) {
		m_sqlite_ = NULL;
		return false;
	}

	if (sqlite3_exec(m_sqlite_, "PRAGMA encoding = \"UTF-8\";", NULL, NULL, NULL) == SQLITE_OK) {
		m_set_encoding = true;
	}
	m_path_ = path;
	return true;
}

void CSqlite::Close()
{
	if (IsOpen()) {
		sqlite3_close(m_sqlite_);
		m_sqlite_ = NULL;
	}
	m_path_.clear();
	m_set_encoding = false;
}

bool CSqlite::BeginTrans()
{
	m_trans_lock.Acquire();
	return Query("BEGIN;");
}

bool CSqlite::Commit()
{
	bool bret = Query("COMMIT;");
	m_trans_lock.Release();
	return bret;
}

bool CSqlite::Rollback()
{
	bool bret = Query("ROLLBACK;");
	m_trans_lock.Release();
	return bret;
}

bool CSqlite::Query(const char* query)
{
	bool bret = false;
	m_result.clear();

	m_error_code_ = SQLITE_OK;
	m_error_msg_.clear();

	if (IsOpen() && query)
	{
		sqlite3_stmt *stmt = NULL;  
		
		/* use the statement as many times as required */  
		if (sqlite3_prepare_v2(m_sqlite_, query, lstrlenA(query), &stmt, NULL) == SQLITE_OK) {

			bret = true;

			int col_count = sqlite3_column_count(stmt);
			for (int i = 0; i < col_count; i++) {
				m_result.add_col(sqlite3_column_name(stmt, i)); 
			}
			/* bind any parameter values */
			//sqlite3_bind_xxx( stmt, param_idx, param_value... );  
			//sqlite3_bind_int(stmt, );

			/* execute statement and step over each row of the result set */
			int step_ret;
			while ((step_ret = sqlite3_step(stmt)) != SQLITE_DONE) {
				if (step_ret != SQLITE_ROW) {
					// error !
					bret = false;
					GetError();
					break;
				} else {
					int irow = -1;
					for (int i = 0; i < col_count; i++) {

						sqlite_row_data rd = {0};

						int type = (ColType)sqlite3_column_type(stmt, i);

						rd.byteslen = sqlite3_column_bytes(stmt, i);

						if (type >= kCTInteger || type <= kCTNULL) {
							rd.type = (ColType)type;
							//SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB, SQLITE_NULL
							/* extract column values from the current result row */
							switch (rd.type) {
							case kCTInteger:
								rd.int64_ = sqlite3_column_int64(stmt, i);
								//rd.int32_ = sqlite3_column_int(stmt, i);
								break;
							case kCTFloat:
								rd.double_ = sqlite3_column_double(stmt, i);
								break;
							case kCTString:
								rd.string_ = (char *)malloc(rd.byteslen + 1);
								memcpy(rd.string_, sqlite3_column_text(stmt, i), rd.byteslen);
								rd.string_[rd.byteslen] = 0;
								break;
							case kCTBLOB:
								rd.blob_ = (unsigned char *)malloc(rd.byteslen);
								memcpy(rd.blob_, sqlite3_column_blob(stmt, i), rd.byteslen);
								break;
							case kCTNULL:
								break;
							default:
								break;
							}
						}
					
						irow = m_result.add_row(i, &rd, irow);

					}	//for (int i = 0; i < col_count
				}
			}

			/* reset the statement so it may be used again */
			// sqlite3_reset(stmt);  
			// sqlite3_clear_bindings(stmt);  /* optional */  

			/* destroy and release the statement */  
			sqlite3_finalize(stmt);  
			stmt = NULL;
		} else {
			GetError();
		}

		/*if (sqlite3_exec(m_sqlite_, query, _callback_exec, &m_result, &err_msg) == SQLITE_OK) {
			bret = true;
		} else {
			wchar_t *wmsg = NULL;
			if (lo_C2W(&wmsg, err_msg)) {
				m_error_msg_ = wmsg;
				free(wmsg);
			}
			sqlite3_free(err_msg);
		}*/
	}
	return bret;
}
