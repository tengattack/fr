cmake_minimum_required(VERSION 3.12)
project(FrLibrary)

# 设置编译选项
set(CMAKE_GENERATOR_PLATFORM x86)

set(SOURCES
     cutui/base/base64.cc
     cuteui/base/md5.cc
     cuteui/base/sha1_portable.cc
     cuteui/base/counter.cc
     cuteui/base/windows_version.cc
     cuteui/base/at_exit.cc
     cuteui/base/debug/alias.cc
     cuteui/base/debug/debugger.cc
     cuteui/base/debug/debugger_win.cc
     cuteui/base/debug/stack_trace.cc
     cuteui/base/debug/stack_trace_win.cc
     cuteui/base/win/event_trace_provider.cc
     cuteui/base/lock.cc
     cuteui/base/lock_impl_win.cc
     cuteui/base/event.cc
     cuteui/base/callback.cc
     cuteui/base/lazy_instance.cc
     cuteui/base/memory/singleton.cc
     cuteui/base/threading/platform_thread.cc
     cuteui/base/rand_util.cc
     cuteui/base/rand_util_win.cc
     cuteui/base/vlog.cc
     cuteui/base/logging.cc
     cuteui/base/logging_win.cc
     cuteui/base/json/values.cc
     cuteui/base/json/values_op.cc
     cuteui/base/json/json_reader.cc
     cuteui/base/json/json_writer.cc
     cuteui/base/json/string_escape.cc
     cuteui/base/file/file.cc
     cuteui/base/file/filedata.cc
     cuteui/base/file/file_path.cc
     cuteui/base/file/file_path_constants.cc
     cuteui/base/string/string_util.cc
     cuteui/base/string/stringprintf.cc
     cuteui/base/string/string_number_conversions.cc
     cuteui/base/string/utf_string_conversion_utils.cc
     cuteui/base/string/string_piece.cc
     cuteui/base/string/utf_string_conversions.cc
     cuteui/base/string/string_split.cc
     cuteui/base/operation/fileselect.cc
     cuteui/base/operation/threadpool.cc
     cuteui/base/third_party/dmg_fp/dtoa.cc
     cuteui/base/third_party/dmg_fp/g_fmt.cc
     cuteui/base/third_party/icu/icu_utf.cc
     cuteui/base/third_party/modp_b64/modp_b64.cc
     common/bbiconv.cc
     common/Buffer.cpp   
     common/properties.cpp
     common/strconv.cpp
     common/string_easy_conv.cc
     common/Urlcode.cpp
     common/wiseint.cc
)

# 编译静态库
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# 添加头文件搜索路径
include_directories(common cuteui)