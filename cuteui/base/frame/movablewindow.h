
#ifndef _SNOW_CUTE_MOVABLE_WINDOW_H_
#define _SNOW_CUTE_MOVABLE_WINDOW_H_ 1

#include "../common.h"

namespace view {
	namespace frame {

		class MovableWindow {
		public:

			MovableWindow();

			bool CheckMoving(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		protected:
			POINT m_last_cursor_pt;
			bool m_last_get_pt;
		};

	}
}

#endif