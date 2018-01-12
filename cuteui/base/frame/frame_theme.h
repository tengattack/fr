
#ifndef _SNOW_CUTE_FRAME_THEME_H_
#define _SNOW_CUTE_FRAME_THEME_H_ 1

#include "../common.h"

namespace view{
	namespace frame{
		namespace theme{

			extern void InitTheme();
			extern bool IsThemeEnabled();
			extern bool EnableWindowTheme(HWND hWnd, LPCWSTR lpszAppName, LPCWSTR lpszClassList, LPCWSTR lpszIdList);

		};
	};
};

#endif