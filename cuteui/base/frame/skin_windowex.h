
#ifndef _SNOW_CUTE_SKIN_WINDOWEX_H_
#define _SNOW_CUTE_SKIN_WINDOWEX_H_ 1

#include "frame_window.h"

#include "../common.h"
#include "skin_data.h"

namespace view{
	namespace frame{
		namespace skin{
			class WindowEx{
			public:
				static bool IsDrawMaximizeBox(CWindow *pWindow);
				static bool IsDrawMinimizeBox(CWindow *pWindow);
				static void GetBorderSize(CWindow *pWindow, SIZE* pSize);
				static int GetCaptionHeight(CWindow *pWindow);
				static bool HasMenu(CWindow *pWindow);
				static void GetScreenRect(CWindow *pWindow, RECT* pRect);
			};
		};
	};
};

#endif