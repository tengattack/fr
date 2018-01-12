
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "check.h"

#include "base/view_gdiplus.h"

namespace view {
	namespace frame {

		CCheck::CCheck()
			: CBaseButtonOwnerDraw(kStyleCheckbox)
		{
		}
		CCheck::~CCheck()
		{
		}

		bool CCheck::Create3State(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle)
		{
			//暂时没有处理
			return CreateButton(hParent, lpszText, id, x, y, width, height, dwStyle);
		}

		bool CCheck::CreateCheck(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle)
		{
			return CreateButtonOwnerDraw(hParent, lpszText, id, x, y, width, height, dwStyle);
		}
		
	};
};