

#ifndef _SNOW_CUTE_BASE_BUTTON_H_
#define _SNOW_CUTE_BASE_BUTTON_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"

namespace view{
	namespace frame{
		
		class CBaseButton : public CChild{

		public:
			CBaseButton();
			~CBaseButton();

			bool CreateButton(HWND hParent, LPCWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON);

			bool GetCheck();
			void SetCheck(bool check);

#if 0
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
#endif

		};
	};
};

#endif