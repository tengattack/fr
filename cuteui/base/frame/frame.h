
#ifndef _SNOW_CUTE_FRAME_H_
#define _SNOW_CUTE_FRAME_H_ 1

#include "frame_window.h"

#include "../common.h"

namespace view{
	namespace frame{

		CWindow* GetWindow(HWND hWnd);
		void InsertWindow(HWND hWnd, CWindow* pWindow);
		void DeleteWindow(CWindow* pWindow);

	};
};

#endif