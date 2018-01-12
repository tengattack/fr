
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "base/frame/skin_data.h"

#include "radio.h"

namespace view {
	namespace frame {

		CRadio::CRadio()
			: CBaseButtonOwnerDraw(kStyleRadio)
		{
		}
		CRadio::~CRadio()
		{
		}

		bool CRadio::CreateRadio(HWND hParent, LPWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, bool bGroupFirst, DWORD dwStyle)
		{
			return CreateButtonOwnerDraw(hParent, lpszText, id, x, y, width, height, dwStyle | (bGroupFirst ? WS_GROUP : NULL));
		}

	};
};