

#ifndef _SNOW_CUTE_RADIO_H_
#define _SNOW_CUTE_RADIO_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "basebutton_ownerdraw.h"

namespace view {
	namespace frame {
		
		class CRadio : public CBaseButtonOwnerDraw {

		public:
			CRadio();
			~CRadio();

			bool CreateRadio(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, bool bGroupFirst = false, DWORD dwStyle = WS_CHILD | WS_TABSTOP | BS_AUTORADIOBUTTON);

		};
	};
};

#endif