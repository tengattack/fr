
#ifndef _TA_COUNTER_H_
#define _TA_COUNTER_H_ 1
#pragma once

#include "lock.h"

namespace base {
	class Counter {

	public:

		friend class ::AutoLock;

		Counter(int value);
		virtual ~Counter();

		virtual void Set(int value);
		virtual void Add(int value);
		virtual int Get();

		void Increment() {
			Add(1);
		}

		void Decrement() {
			Add(-1);
		}

		void Subtract(int value) {
			Add(-value);
		}

	protected:

		Lock m_lock;
		void* m_ptr;
	};
};

#endif