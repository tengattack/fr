

#ifndef _SNOW_CUTE_CHECK_H_
#define _SNOW_CUTE_CHECK_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "basebutton_ownerdraw.h"

namespace view {
	namespace frame {
		
		class CCheck : public CBaseButtonOwnerDraw {

		public:
			CCheck();
			~CCheck();

			bool Create3State(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP | BS_AUTO3STATE);

			bool CreateCheck(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX);

		};
	};
};

#endif