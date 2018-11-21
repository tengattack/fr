
//add _NO_GDIPLUS_
#ifndef _CRT_RAND_S
#define _CRT_RAND_S
#endif

#define NOMINMAX 1
#undef min
#undef max

#include "base64.cc"
#include "md5.cc"
#include "sha1_portable.cc"
#include "counter.cc"

#include "windows_version.cc"

#include "at_exit.cc"

// debug
#pragma comment(lib, "dbghelp.lib")
#include "debug/alias.cc"
#include "debug/debugger.cc"
#include "debug/debugger_win.cc"
#include "debug/stack_trace.cc"
#include "debug/stack_trace_win.cc"

#pragma comment(lib, "advapi32.lib")
#include "win/event_trace_provider.cc"

#include "lock.cc"
#include "lock_impl_win.cc"
#include "event.cc"
#include "callback.cc"
#include "lazy_instance.cc"
#include "memory/singleton.cc"
#include "threading/platform_thread.cc"

#include "rand_util.cc"
#include "rand_util_win.cc"

#include "vlog.cc"
#include "logging.cc"
#include "logging_win.cc"

#include "json/values.cc"
#include "json/values_op.cc"
#include "json/json_reader.cc"
#include "json/json_writer.cc"
#include "json/string_escape.cc"

#include "file/file.cc"
#include "file/filedata.cc"
#include "file/file_path_constants.cc"
#include "file/file_path.cc"

#include "string/string_util.cc"
#include "string/stringprintf.cc"
#include "string/string_number_conversions.cc"
#include "string/utf_string_conversion_utils.cc"
#include "string/string_piece.cc"
#include "string/utf_string_conversions.cc"
#include "string/string_split.cc"

//operation
#include "operation/fileselect.cc"
#include "operation/threadpool.cc"

//db
//#include "db/ado2.cc"
#ifdef USE_SQLITE
#include "db/sqlite.cc"
#endif

#include "third_party\dmg_fp\dtoa.cc"
#include "third_party\dmg_fp\g_fmt.cc"
#include "third_party\icu\icu_utf.cc"

#include "third_party\modp_b64\modp_b64.cc"

//#define max(a,b)    (((a) > (b)) ? (a) : (b))
//#define min(a,b)    (((a) < (b)) ? (a) : (b))
