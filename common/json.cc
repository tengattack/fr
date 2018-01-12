
#include "json.h"

namespace tengattack {
	namespace base {

JSONReader::JSONReader()
{
}

JSONReader::~JSONReader()
{
}

std::string& replace_all(std::string& str, const std::string& old_value, const std::string& new_value)
{   
	while (true)
	{
		std::string::size_type pos(0);
		if ((pos=str.find(old_value)) != std::string::npos)
			str.replace(pos, old_value.length(), new_value);
		else
			break;
	}
	return str;
}

void JSONReader::ProcessValue(LPCSTR value, int len, std::string& out)
{
	if (len >= 2)
	{
		switch (value[0])
		{
		case '\'':
		case '\"':
			out = std::string(value + 1, len - 2);
			break;
		default:
      for (int i = len - 1; i > 0; i--) {
        if (value[i] == ' ' || value[i] == '\t' || value[i] == '\r' || value[i] == '\n') {
          len--;
        } else {
          break;
        }
      }
			out = std::string(value, len);
		}
	} else {
		out = std::string(value, len);
	}
	replace_all(out, "\\/", "/");
}

bool JSONReader::Read(LPCSTR json)
{
	int len = lstrlenA(json);
	if (len <= 2)	// {}
	{
		return false;
	}

	//只能解析一级结构
	int begin_ = 0;
	bool readed = false;
	const char *name_ = NULL;
	const char *value_ = NULL;

	std::string name, value;
	int i;
	for (i = 0; i < len; i++)
	{
		if (json[i] == '\t' || json[i] == '\r' || json[i] == '\n' || json[i] == ' ') continue;
		if (json[i] == '{' && !begin_)
		{
			readed = true;
			begin_++;
			continue;
		}
		if (begin_)
		{
			//结束
			if (json[i] == '}')
			{
				begin_--;
				if (begin_ == 0)
				{
					if (value_)
					{
						ProcessValue(value_, i - (value_ - json), value);
						m_properties.Put(name.c_str(), value.c_str());

						name_ = NULL;
						value_ = NULL;
					}
					break;
				}
			} else if (json[i] == ':') {
				if (name_)
				{
					ProcessValue(name_, i - (name_ - json), name);
					value_ = json + i + 1;
				}
			} else if (json[i] == ',') {
				if (value_)
				{
					ProcessValue(value_, i - (value_ - json), value);
					m_properties.Put(name.c_str(), value.c_str());

					name_ = NULL;
					value_ = NULL;
				}
			} else if (!name_) {
				name_ = json + i;
			}
		}
	}

	return readed;
}

void JSONReader::Get(LPCSTR name, std::string& data)
{
	m_properties.Get(name, data);
}
	
	};
};