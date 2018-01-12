
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "frame.h"
#include "frame_window.h"
#include "frame_skia.h"

#include "mainwindow.h"
#include "skin_windowex.h"
#include "base/frame/skin_data.h"

namespace view{
	namespace frame{
		CMainWindow::CMainWindow()
			: m_skin(this)
		{
		}

		CMainWindow::~CMainWindow()
		{
		}

		void CMainWindow::InitMainWindow()
		{
			CDialog::RegisterClass();
			view::RegisterClass(kMainWindowClassName, 
					m_big_icon,
					m_small_icon);
		}

		int CMainWindow::AddStaticText(unsigned short id, int x, int y, int width, int height, LPCWSTR lpszText, bool show_, int text_size)
		{
			return CDialog::AddStaticText(id, x, skin::WindowEx::GetCaptionHeight(this) + y, width, height, lpszText, show_, text_size);
		}

		void CMainWindow::CreateMemoryDC()
		{
			SkResize(m_width, m_height + skin::WindowEx::GetCaptionHeight(this));
			RedrawBackground();

			//需要触发事件，通知所有子窗口修改背景
		}

		void CMainWindow::RedrawBackground()
		{
			SkCanvas canvas(m_skbitmap);

			skin::CSkinData::OnDrawBackground(canvas, this, m_width, m_height + skin::WindowEx::GetCaptionHeight(this));

			OnDrawText(canvas, (SkColor)skin::CSkinData::m_si.main_color.GetValue());

			SendRedrawBackgroundMessage(m_hWnd);
		}

		bool CMainWindow::CreateMainWindow(LPCWSTR lpszTitle, bool show)
		{
			int cx = GetSystemMetrics(SM_CXSCREEN);
			int cy = GetSystemMetrics(SM_CYSCREEN);
			
			if (Create(NULL, kMainWindowClassName, lpszTitle, 
							 WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_THICKFRAME		| \
                             WS_MINIMIZEBOX,
				(cx - GetInitialWidth()) / 2, (cy - GetInitialHeight()) / 2,
				GetInitialWidth(), GetInitialHeight()))
			{
				global::pMainWindow = this;
				
				m_width = GetInitialWidth();
				m_height = GetInitialHeight();
				CreateMemoryDC();

				m_skin.UpdateStyle();

				if (show) Show();
				return true;
			} else {
				return false;
			}

			return true;
		}

		LRESULT CALLBACK CMainWindow::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			//Skin Proc
			int lResult = m_skin.OnWndProc(message, wParam, lParam, handled);
			if (handled)
			{
				return lResult;
			}

			switch (message)
			{
			case WM_TA_REDRAW_BACKGROUND:
				handled = true;
				m_skin.OnNcPaint(true);
				break;
			case WM_SIZE:
				{
					if (!IsMinimized()) {
						handled = true;
						m_width = LOWORD(lParam); // width of client area
						m_height = HIWORD(lParam) + skin::WindowEx::GetCaptionHeight(this); // height of client area
						
						//resize
						OnSize(m_width, m_height);

						CreateMemoryDC();
					}
				}
				break;
			case WM_CTLCOLORSTATIC:
				handled = true;
				{
					HDC hdc = (HDC)wParam;
					HWND hChildWnd = (HWND)lParam;
					RECT rc, rcParent;
					if (::GetWindowRect(hChildWnd, &rc) && ::GetWindowRect(m_hWnd, &rcParent))
					{
						handled = true;

						//::SetTextColor(hdc, skin::CSkinData::m_si.main_color.ToCOLORREF());

						SetBkMode(hdc, TRANSPARENT);
						/*BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
							m_hMemDC, rc.left - rcParent.left, rc.top - rcParent.top, SRCCOPY);*/
						doSkPaint(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, rc.left - rcParent.left, rc.top - rcParent.top);
					}
					//we handled it.
					return (LRESULT)GetStockObject(NULL_BRUSH);
				}
				break;
			case WM_ERASEBKGND:
				{
					handled = true;
					HDC hdc = (HDC)wParam;
					int cheight = skin::WindowEx::GetCaptionHeight(this);
					//::BitBlt(hdc, 0, 0, m_width, m_height - height, m_hMemDC, 0, height, SRCCOPY);
					doSkPaint(hdc, 0, 0, m_width, m_height - cheight, 0, cheight);
				}
				break;
			case WM_PAINT:
				{
					handled = true;
					PAINTSTRUCT ps = {0};
					HDC hdc = BeginPaint(m_hWnd, &ps);
					int height = skin::WindowEx::GetCaptionHeight(this);
					//::BitBlt(hdc, 0, 0, m_width, m_height - height, m_hMemDC, 0, height, SRCCOPY);
					doSkPaint(hdc, 0, 0, m_width, m_height - height, 0, height);
					EndPaint(m_hWnd, &ps);
				}
				break;
			}
			if (handled) return 0;
			return CDialog::OnWndProc(message, wParam, lParam, handled);
		}

		WindowType CMainWindow::GetType()
		{
			return kWTMain;
		}
	};
};