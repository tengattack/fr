
#include "event.h"

Event::Event()
	: event_(NULL)
{
}

Event::~Event()
{
	Close();
}

bool Event::Create(bool manual_reset, bool initial_state, LPCWSTR name)
{
	event_ = CreateEventW(NULL, manual_reset, initial_state, name);
	return (event_ != NULL);
}

void Event::Close()
{
	if (event_) {
		CloseHandle(event_);
		event_ = NULL;
	}
}

bool Event::Reset()
{
  if (event_ == NULL) {
    return false;
  }
	return static_cast<bool>(ResetEvent(event_));
}

bool Event::Set()
{
  if (event_ == NULL) {
    return false;
  }
	return static_cast<bool>(SetEvent(event_));
}

void Event::Wait(uint32 milliseconds)
{
  if (event_ == NULL) {
    return;
  }
	WaitForSingleObject(event_, milliseconds);
}

