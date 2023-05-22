#include "base/stdafx.h"

#include "base/threading/platform_thread.h"

namespace base {

/* static */
void PlatformThread::YieldCurrentThread() {
  ::Sleep(0);
}

}  // namespace base
