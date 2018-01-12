
#include "json-config.h"

#include "json/json_writer.h"
#include "json/json_reader.h"
#include "json/values.h"
#include "string/string_tokenizer.h"

#include "file/file.h"
#include "file/filedata.h"

namespace base {

	bool JsonToValue(std::string& json, Value **val)
	{
#ifdef DEBUG
		int err_code = 0;
		std::string err_msg;

		Value* v = base::JSONReader::ReadAndReturnError(json, true, &err_code, &err_msg);
#else
		Value* v = base::JSONReader::Read(json, true);
#endif
		if (v) {
			*val = v;
			return true;
		} else {
#ifdef DEBUG
			printf("Read failed, error code: %d\n%s\n", err_code, err_msg.c_str());
#endif
			return false;
		}

	}
	
	bool ReadJsonFile(LPCTSTR filepath, Value **val)
	{
		bool bret = false;
		base::CFile file;
		if (file.Open(base::kFileRead, filepath)) {
			base::CFileData fd;
			if (fd.Read(file)) {
				std::string strraw;
				fd.ToUtf8Text(strraw);

				if (strraw.length()) {

					std::string strsjon;
					//处理注释
					StringTokenizer t(strraw, "\n");
					bool in_note = false;
					//t.set_quote_chars("\"");

					//这里没有检查注释在引号中的情况
					while (t.GetNext()) {

						std::string line = t.token();

						while (true) {
							const char *base = line.c_str();
							const char *note1 = NULL;
							const char *note2 = in_note ? strstr(base, "*/") : strstr(base, "/*");
					
							if (!in_note) note1 = strstr(base, "//");

							if (!in_note) {
								if (note1 && (note1 < note2 || !note2)) {
									// //注释在前面
									strsjon += line.substr(0, note1 - base);
								} else if (note2) {
									strsjon += line.substr(0, note2 - base);

									//同行
									const char *note3 = strstr(note2 + 2, "*/");
									if (note3) {
										line = note3 + 2;
										continue;
									} else {
										in_note = true;
									}
								} else {
									strsjon += line;
								}
							} else if (note2) {
								strsjon += line.substr(note2 + 2 - base);
								in_note = false;
							}

							break;
						}

						strsjon += "\n";
					}

					bret = JsonToValue(strsjon, val);
				}
			}
			file.Close();
		}
		return bret;
	}

	CJSONConfig::CJSONConfig()
		: m_changed(false)
		, m_value(NULL)
	{
	}

	CJSONConfig::~CJSONConfig()
	{
		Save();
		if (m_value) {
			delete m_value;
			m_value = NULL;
		}
	}

	bool CJSONConfig::Save()
	{
		bool saved_ = true;
		if (m_changed) {
			base::CFile m_file;
			if (m_file.Open(base::kFileCreate, m_path.c_str())) {
				//m_file.Write();
				if (m_value) {
					std::string json;
					base::JSONWriter::Write(m_value, true, &json);
					if (json.length() > 0) {
						m_file.Write((BYTE *)"\xEF\xBB\xBF", 3); //UTF-8
						saved_ = m_file.Write((BYTE *)json.c_str(), json.length());
						m_changed = false;
					}
				}
			} else {
				saved_ = false;
			}
		}
		return saved_;
	}

	bool CJSONConfig::SetString(const std::string& path,
							const std::string& in_value)
	{
		if (m_value) {
			std::string o_value;
			if (m_value->GetString(path, &o_value)) {
				if (o_value == in_value) {
					return true;
				}
			}
			m_changed = true;
			m_value->SetString(path, in_value);
			return true;
		}
		return false;
	}

	bool CJSONConfig::SetString(const std::string& path,
							const string16& in_value)
	{
		if (m_value) {
			string16 o_value;
			if (m_value->GetString(path, &o_value)) {
				if (o_value == in_value) {
					return true;
				}
			}
			m_changed = true;
			m_value->SetString(path, in_value);
			return true;
		}
		return false;
	}

	
	bool CJSONConfig::SetBoolean(const std::string& path, bool in_value)
	{
		if (m_value) {
			bool o_value = false;
			if (m_value->GetBoolean(path, &o_value)) {
				if (o_value == in_value) {
					return true;
				}
			}
			m_changed = true;
			m_value->SetBoolean(path, in_value);
			return true;
		}
		return false;
	}

	bool CJSONConfig::SetInteger(const std::string& path, int in_value)
	{
		if (m_value) {
			int o_value = 0;
			if (m_value->GetInteger(path, &o_value)) {
				if (o_value == in_value) {
					return true;
				}
			}
			m_changed = true;
			m_value->SetInteger(path, in_value);
			return true;
		}
		return false;
	}

	bool CJSONConfig::GetString(const std::string& path,
							string16* out_value, LPCWSTR default_value)
	{
		if (m_value) {
			if (m_value->GetString(path, out_value))
			{
				return true;
			} else {
				*out_value = default_value;
			}
		}
		return false;
	}

	bool CJSONConfig::GetString(const std::string& path,
							std::string* out_value, LPCSTR default_value)
	{
		if (m_value) {
			if (m_value->GetString(path, out_value))
			{
				return true;
			} else {
				*out_value = default_value;
			}
		}
		return false;
	}

	bool CJSONConfig::GetBoolean(const std::string& path, bool* out_value, bool default_value)
	{
		if (m_value) {
			if (m_value->GetBoolean(path, out_value))
			{
				return true;
			} else {
				*out_value = default_value;
			}
		}
		return false;
	}

	bool CJSONConfig::GetInteger(const std::string& path, int* out_value, int default_value)
	{
		if (m_value) {
			if (m_value->GetInteger(path, out_value))
			{
				return true;
			} else {
				*out_value = default_value;
			}
		}
		return false;
	}

  void CJSONConfig::SetPath(LPCTSTR path_)
	{
		m_path = path_;

		bool bread = false;
		if (ReadJsonFile(path_, (Value **)&m_value)) {
			if (m_value && m_value->GetType() == Value::TYPE_DICTIONARY) {
				bread = true;
			}
		}

		if (!bread) {
			if (m_value) {
				delete m_value;
				m_value = NULL;
			}

			m_value = new DictionaryValue;
			m_changed = true;
		}
	}
}