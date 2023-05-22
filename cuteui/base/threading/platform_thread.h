
#ifndef BASE_THREADING_PLATFORM_THREAD_H_
#define BASE_THREADING_PLATFORM_THREAD_H_

#include "base/basictypes.h"

#include "base/base_export.h"

namespace base {

// A namespace for low-level thread functions.
class BASE_EXPORT PlatformThread {
public:
	// Yield the current thread so another thread can be scheduled.
	static void YieldCurrentThread();
private:
	DISALLOW_IMPLICIT_CONSTRUCTORS(PlatformThread);
};

}  // namespace base

#endif  // BASE_THREADING_PLATFORM_THREAD_H_