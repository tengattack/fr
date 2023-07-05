cmake_minimum_required(VERSION 3.12)
project(FrLibrary)

# 设置编译选项
set(CMAKE_GENERATOR_PLATFORM x86)

set(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cuteui/base)
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)

set(SOURCES
     ${BASE_DIR}/base64.cc
     ${BASE_DIR}/md5.cc
     ${BASE_DIR}/sha1_portable.cc
     ${BASE_DIR}/counter.cc
     ${BASE_DIR}/windows_version.cc
     ${BASE_DIR}/at_exit.cc
     ${BASE_DIR}/debug/alias.cc
     ${BASE_DIR}/debug/debugger.cc
     ${BASE_DIR}/debug/debugger_win.cc
     ${BASE_DIR}/debug/stack_trace.cc
     ${BASE_DIR}/debug/stack_trace_win.cc
     ${BASE_DIR}/win/event_trace_provider.cc
     ${BASE_DIR}/lock.cc
     ${BASE_DIR}/lock_impl_win.cc
     ${BASE_DIR}/event.cc
     ${BASE_DIR}/callback.cc
     ${BASE_DIR}/lazy_instance.cc
     ${BASE_DIR}/memory/singleton.cc
     ${BASE_DIR}/threading/platform_thread.cc
     ${BASE_DIR}/rand_util.cc
     ${BASE_DIR}/rand_util_win.cc
     ${BASE_DIR}/vlog.cc
     ${BASE_DIR}/logging.cc
     ${BASE_DIR}/logging_win.cc
     ${BASE_DIR}/json/values.cc
     ${BASE_DIR}/json/values_op.cc
     ${BASE_DIR}/json/json_reader.cc
     ${BASE_DIR}/json/json_writer.cc
     ${BASE_DIR}/json/string_escape.cc
     ${BASE_DIR}/file/file.cc
     ${BASE_DIR}/file/filedata.cc
     ${BASE_DIR}/file/file_path.cc
     ${BASE_DIR}/file/file_path_constants.cc
     ${BASE_DIR}/string/string_util.cc
     ${BASE_DIR}/string/stringprintf.cc
     ${BASE_DIR}/string/string_number_conversions.cc
     ${BASE_DIR}/string/utf_string_conversion_utils.cc
     ${BASE_DIR}/string/string_piece.cc
     ${BASE_DIR}/string/utf_string_conversions.cc
     ${BASE_DIR}/string/string_split.cc
     ${BASE_DIR}/operation/fileselect.cc
     ${BASE_DIR}/operation/threadpool.cc
     ${BASE_DIR}/third_party/dmg_fp/dtoa.cc
     ${BASE_DIR}/third_party/dmg_fp/g_fmt.cc
     ${BASE_DIR}/third_party/icu/icu_utf.cc
     ${BASE_DIR}/third_party/modp_b64/modp_b64.cc
     ${COMMON_DIR}/bbiconv.cc
     ${COMMON_DIR}/Buffer.cpp   
     ${COMMON_DIR}/properties.cpp
     ${COMMON_DIR}/strconv.cpp
     ${COMMON_DIR}/string_easy_conv.cc
     ${COMMON_DIR}/Urlcode.cpp
     ${COMMON_DIR}/wiseint.cc
)

# 编译静态库
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# 添加头文件搜索路径
include_directories(common cuteui)