
#include "frame.h"
#include "frame_window.h"
#include "frame_child.h"

#include "../global.h"
#include "../view.h"

namespace view{
	namespace frame{
		CChild::CChild()
			: m_oldWndProc(NULL)
			, m_id(0)
			, m_bHover(false)
		{
		}
		CChild::~CChild()
		{
		}

		void CChild::SubClassWindow()
		{
			m_oldWndProc = (WNDPROC)SetWindowLong(hWnd(), GWL_WNDPROC, (LONG)WndProc);
		}

		int CChild::OnMouseMove(bool movein)
		{
			return 0;
		}

		int CChild::OnMouseMove(bool movein, int x, int y)
		{
			return 0;
		}
		
		int CChild::OnMouseLeave()
		{
			return 0;
		}

		SkBitmap* CChild::GetParentSkBitmap()
		{
			HWND hParentWnd = ::GetParent(m_hWnd);
			if (hParentWnd)
				return (SkBitmap *)SendMessage(hParentWnd, WM_TA_GET_SKBITMAP, NULL, NULL);
			else
				return NULL;
		}

		LRESULT CALLBACK CChild::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			if (handled)
				return 0;

			switch (message)
			{
			case WM_MOUSEMOVE:
				{
					bool movein = !m_bHover;
					m_bHover = true;
					OnMouseMove(movein);
					OnMouseMove(movein, (int)LOWORD(lParam), (int)HIWORD(lParam));

					if (movein)
					{
						TRACKMOUSEEVENT tme;
						tme.cbSize = sizeof(tme);
						tme.dwFlags = TME_LEAVE;
						tme.hwndTrack = m_hWnd;
						tme.dwHoverTime = 0;
						TrackMouseEvent(&tme);
					}
				}
				break;
			case WM_MOUSELEAVE:
				m_bHover = false;
				OnMouseLeave();
				break;
			}
			handled = true;
			return m_oldWndProc(m_hWnd, message, wParam, lParam);
		}
	};
};
