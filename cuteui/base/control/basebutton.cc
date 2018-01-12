
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "basebutton.h"

#include "base/view_gdiplus.h"

namespace view{
	namespace frame{
		
		CBaseButton::CBaseButton()
		{
		}
		CBaseButton::~CBaseButton()
		{
		}

#if 0
		LRESULT CALLBACK CBaseButton::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_ERASEBKGND:
				{
					HWND hParentWnd = GetParent();
					HDC hParentMemDC = (HDC)SendMessage(hParentWnd, WM_TA_GET_MEMORY_HDC, NULL, NULL);
					if (hParentMemDC)
					{
						HDC hdc = (HDC)wParam;
						RECT rc, rcParent;
						if (::GetWindowRect(m_hWnd, &rc) && ::GetWindowRect(hParentWnd, &rcParent))
						{
							handled = true;
							SetBkMode(hdc, TRANSPARENT);
							BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
								hParentMemDC, rc.left - rcParent.left, rc.top - rcParent.top, SRCCOPY);
						}
						//we handled it.
						return TRUE;
					}

				}
				break;
			}
			return CChild::OnWndProc(message, wParam, lParam, handled);
		}
#endif

		bool CBaseButton::CreateButton(HWND hParent, LPCWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (Create(hParent, L"Button", lpszText, dwStyle,
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

		bool CBaseButton::GetCheck()
		{
			//Button_GetCheck
			return ((::SendMessage(m_hWnd, BM_GETCHECK, NULL, NULL) & BST_CHECKED) != NULL);
		}

		void CBaseButton::SetCheck(bool check)
		{
			//wParam
			//The check state. This parameter can be one of the following values.
			::SendMessage(m_hWnd, BM_SETCHECK, check ? BST_CHECKED : BST_UNCHECKED, NULL);
		}

	};
};