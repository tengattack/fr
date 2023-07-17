
#ifndef BASE_BUILD_CONFIG_H_
#define BASE_BUILD_CONFIG_H_
#pragma once

#define COMPILER_MSVC		1
#define OS_WIN				1
#define WCHAR_T_IS_UTF16	1

#define HANDLE_EINTR(x) (x)
#define IGNORE_EINTR(x) (x)

#define BUILDFLAG(name) 0

#endif