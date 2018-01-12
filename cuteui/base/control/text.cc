
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "text.h"

namespace view{
	namespace frame{
		
		CText::CText()
		{
		}
		CText::~CText()
		{
		}

		bool CText::CreateText(HWND hParent, LPCWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (CreateEx(hParent, L"Edit", lpszText, dwStyle, WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE,
				x, y,
				width, height, (HMENU)id))
			{
				SetID(id);
				SubClassWindow();

				Show();
				return true;
			} else {
				return false;
			}

			return true;
		}

		void CText::ReplaceSel(LPCWSTR lpszText, bool undone_)
		{
			::SendMessage(m_hWnd, EM_REPLACESEL, (undone_ ? TRUE : FALSE), (LPARAM)lpszText);
		}

		/*int CText::OnMouseMove(bool movein)
		{
			if (movein) SetText(L"Move");
			return 0;
		}
		
		int CText::OnMouseLeave()
		{
			SetText(L"Leave");
			return 0;
		}*/

		LRESULT CALLBACK CText::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			return CChild::OnWndProc(message, wParam, lParam, handled);
		}
	};
};