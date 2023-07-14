
#ifndef _SNOW_CUTE_COMMON_H_
#define _SNOW_CUTE_COMMON_H_ 1

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
// Windows 头文件:
#include <windows.h>

//解除预定义 CreateDialogW
#ifdef CreateDialog
#undef CreateDialog
#endif

#ifdef RegisterClass
#undef RegisterClass
#endif

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <Commctrl.h>
#pragma comment(lib, "Comctl32.lib") 

//tcmalloc
#include <base/tcmalloc.h>

//std
#include <string>
#include <iostream>

//gdiplus
#ifndef _NO_GDIPLUS_
#include <atlimage.h>
using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib") 
#endif

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


#endif
