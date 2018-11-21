#include "bbiconv.cc"
#include "Buffer.cpp"
#include "properties.cpp"
#include "strconv.cpp"
#include "string_easy_conv.cc"
#include "Urlcode.cpp"
//#include "ThreadPool.cpp" -> <base/operation/threadpool.h>
#include "wiseint.cc"
#include "json.cc"
// #include "StackWalker.cpp" -> <base/debug/stack_trace.h>

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
