
#include "frame.h"
#include "frame_window.h"
#include "frame_child.h"
#include "frame_custom.h"

#include "../global.h"
#include "../view.h"


namespace view{
	namespace frame{
		CCustom::CCustom()
			: m_width(0)
			, m_height(0)
			, m_need_update(true)
		{
		}
		CCustom::~CCustom()
		{
		}

		void CCustom::CreateMemoryDC()
		{
			SkResize(m_width, m_height);
			m_need_update = true;
		}

		LRESULT CALLBACK CCustom::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			if (handled)
			{
				return 0;
			}
			switch (message)
			{
			case WM_TA_GET_SKBITMAP:
				handled = true;
				return (LRESULT)&m_skbitmap;
				break;
			case WM_TA_REDRAW_BACKGROUND:
				handled = true;
				UIUpdate(true);
				break;
			case WM_MOUSEMOVE:
				{
					handled = true;
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
				handled = true;
				m_bHover = false;
				OnMouseLeave();
				break;
			case WM_SIZE:
				{
					handled = true;
					m_width = LOWORD(lParam); // width of client area
					m_height = HIWORD(lParam); // height of client area
					//resize

					OnSize(m_width, m_height);

					CreateMemoryDC();
					UIUpdate();
				}
				break;
			case WM_ERASEBKGND:
				{
					handled = true;

					HDC hdc = (HDC)wParam;
					//::BitBlt(hdc, 0, 0, m_width, m_height, m_hMemDC, 0, 0, SRCCOPY);
					doSkPaint(hdc, 0, 0, m_width, m_height, 0, 0);
				}
				break;
			case WM_PAINT:
				{
					UIUpdate();

					handled = true;
					PAINTSTRUCT ps = {0};
					HDC hdc = BeginPaint(m_hWnd, &ps);
					//::BitBlt(hdc, 0, 0, m_width, m_height, m_hMemDC, 0, 0, SRCCOPY);
					doSkPaint(hdc, 0, 0, m_width, m_height, 0, 0);
					EndPaint(m_hWnd, &ps);
				}
				break;
			}
			//return CChild::OnWndProc(message, wParam, lParam, handled);
			//自定义控件
			return 0;
		}

		void CCustom::RePaint()
		{
			UIUpdate();

			HDC hdc = GetDC(m_hWnd);
			//::BitBlt(hdc, 0, 0, m_width, m_height, m_hMemDC, 0, 0, SRCCOPY);
			doSkPaint(hdc, 0, 0, m_width, m_height, 0, 0);
			ReleaseDC(m_hWnd, hdc);
		}

		void CCustom::UIUpdate(bool force_)
		{
			if (m_need_update || force_)
			{
				m_need_update = false;
			}
		}

		bool CCustom::PaintParent(SkCanvas& canvas)
		{
			return SkiaPaintParent(m_hWnd, canvas);
		}

		void CCustom::OnSize(int width, int height)
		{
		}
	};
};
