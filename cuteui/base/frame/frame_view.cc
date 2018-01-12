
#include "frame.h"
#include "frame_view.h"
#include "frame_window.h"

#include "skin_data.h"

#include "../global.h"
#include "../view.h"

namespace view{
	namespace frame{
		CView::CView()
			: m_text_color(SK_ColorBLACK)
		{
			m_text_color = (SkColor)skin::CSkinData::m_si.view_color.GetValue();
		}
		CView::~CView()
		{
		}

		WindowType CView::GetType()
		{
			return kWTView;
		}

		bool CView::RegisterClass()
		{
			WNDCLASSEX wcex;
			memset(&wcex, 0, sizeof(WNDCLASSEX));

			wcex.cbSize = sizeof(WNDCLASSEX);

			wcex.style			= CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc	= WndProc;
			//wcex.cbClsExtra		= 0;
			//wcex.cbWndExtra		= 0;
			wcex.hInstance		= global::hInstance;
			//wcex.hIcon			= NULL;
			wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);	//(HBRUSH)(COLOR_WINDOW+1);
			//wcex.lpszMenuName	= NULL;
			wcex.lpszClassName	= kViewClassName;
			//wcex.hIconSm		= NULL;

			//如果函数失败，返回值为0
			return (RegisterClassEx(&wcex) != 0);
		}

		bool CView::CreateView(HWND hParent, LPCWSTR lpszText, unsigned short id,
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (Create(hParent, kViewClassName, lpszText, dwStyle,
				x, y,
				width, height, (HMENU)id))
			{
				SetID(id);
				
				m_width = width;
				m_height = height;
				CreateMemoryDC();
				//UIUpdate();

				Show();
				return true;
			} else {
				return false;
			}

			return true;
		}

		int CView::CreateView(CViewContainer *pContainer, LPCWSTR lpszText, unsigned short id, DWORD dwStyle)
		{
			int iView = -1;
			if (pContainer) {
				if (CreateView(pContainer->m_wnd_container, lpszText, id, 
						pContainer->m_view_x, pContainer->m_view_y, pContainer->m_view_width, pContainer->m_view_height, dwStyle)) {
					iView = pContainer->AddView(this);
				}
			}
			return iView;
		}

		void CView::ShowView(bool show_)
		{
			Show(show_);
		}

		void CView::AddWindow(CWindow *pWindow)
		{
			m_window_list.push_back(pWindow);
		}

		int CView::AddStaticText(unsigned short id, int x, int y, int width, int height, LPCWSTR lpszText, bool show_, int text_size)
		{
			int index = CStaticText::AddStaticText(id, x, y, width, height, lpszText, show_, text_size);
			
			//Set Update Flags
			m_need_update = true;

			return index;
		}

		void CView::SetStaticText(int index, LPCWSTR lpszText, bool show_)
		{
			CStaticText::SetStaticText(index, lpszText, show_);
			
			//Set Update Flags
			m_need_update = true;
		}

		void CView::SetTextColor(SkColor color)
		{
			if (m_text_color != color) {
				m_text_color = color;

				//Update
				m_need_update = true;
				UIUpdate();
			}
		}

		void CView::UIUpdate(bool force_)
		{
			if (m_need_update || force_)
			{
				m_need_update = false;

				SkCanvas canvas(m_skbitmap);

				if (!PaintParent(canvas)) {
					canvas.clear(SK_ColorWHITE);
				}

				OnDraw(canvas);
				OnDrawText(canvas, m_text_color);
			}
		}

		void CView::OnDraw(SkCanvas& canvas)
		{
		}

		void CView::Redraw(RECT *rect)
		{
			if (rect) {
				::InvalidateRect(m_hWnd, rect, TRUE);
			} else {
				RECT client = {0};
				::GetClientRect(m_hWnd, &client);
				::InvalidateRect(m_hWnd, &client, TRUE);
			}
		}

		LRESULT CALLBACK CView::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			if (handled)
			{
				return 0;
			}

			switch (message)
			{
			case WM_CTLCOLORSTATIC:
				handled = true;
				{
					HDC hdc = (HDC)wParam;
					HWND hChildWnd = (HWND)lParam;
					RECT rc, rcParent;
					if (::GetClientRect(hChildWnd, &rc) && ::GetClientRect(m_hWnd, &rcParent))
					{
						::ClientToScreen(hChildWnd, (LPPOINT)&rc);
						::ClientToScreen(hChildWnd, &(((LPPOINT)&rc)[1]));
						::ClientToScreen(m_hWnd, (LPPOINT)&rcParent);
						::ClientToScreen(m_hWnd, &(((LPPOINT)&rcParent)[1]));

						handled = true;

						//::SetTextColor(hdc, RGB(SkColorGetR(m_text_color), SkColorGetG(m_text_color), SkColorGetB(m_text_color)));

						SetBkMode(hdc, TRANSPARENT);

						/*BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
							m_hMemDC, rc.left - rcParent.left, rc.top - rcParent.top, SRCCOPY);*/

						doSkPaint(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, rc.left - rcParent.left, rc.top - rcParent.top);
					}
					//we handled it.
					return (LRESULT)GetStockObject(NULL_BRUSH);
				}
				break;
			}

			return CCustom::OnWndProc(message, wParam, lParam, handled);
		}

	};
};
