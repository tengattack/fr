
#include "counter.h"

namespace base {
	Counter::Counter(int value)
	{
		m_ptr = malloc(4);
		if (m_ptr)
		{
			memset(m_ptr, 0, 4);
			Set(value);
		}
	}

	Counter::~Counter()
	{
		if (m_ptr) free(m_ptr);
	}

	void Counter::Set(int value)
	{
		if (m_ptr)
		{
			AutoLock al(m_lock);
			*(int *)m_ptr = value;
		}
	}

	void Counter::Add(int value)
	{
		if (m_ptr)
		{
			AutoLock al(m_lock);
			*(int *)m_ptr += value;
		}
	}

	int Counter::Get()
	{
		if (m_ptr)
		{
			return *(int *)m_ptr;
		} else {
			return 0;
		}
	}
}