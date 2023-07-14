
#ifndef BASE_EVENT_H_
#define BASE_EVENT_H_
#pragma once

#include "base/basictypes.h"
#include <windows.h>

class Event {
 public:

  Event();
  ~Event();

  bool Create(bool manual_reset, bool initial_state, LPCWSTR name = NULL);
  void Close();

  bool Reset();
  bool Set();

  void Wait(uint32 milliseconds = INFINITE);

 private:

  HANDLE event_;

  DISALLOW_COPY_AND_ASSIGN(Event);
};

#endif  // BASE_LOCK_H_