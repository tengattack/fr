
#include "frame.h"
#include "frame_window.h"

#include "base/global.h"
#include "base/view.h"
#include "resource.h"

#include "dialog.h"
#include "skin_windowex.h"
#include "base/frame/skin_data.h"
#include "base/frame/messageloop.h"

namespace view{
	namespace frame{
		HICON CDialog::m_small_icon = NULL;
		HICON CDialog::m_big_icon = NULL;

		CDialog::CDialog()
			: m_width(0)
			, m_height(0)
			, m_looping(false)
			, m_parent(NULL)
		{
		}
		CDialog::~CDialog()
		{
		}

		bool CDialog::RegisterClass()
		{
			m_small_icon = LoadIcon(global::hInstance, MAKEINTRESOURCE(IDI_SMALL));
			m_big_icon = LoadIcon(global::hInstance, MAKEINTRESOURCE(IDI_ICON));
			return (view::RegisterClass(kDialogClassName, 
					m_big_icon,
					m_small_icon) != 0);
		}

		WindowType CDialog::GetType()
		{
			return kWTDialog;
		}

		int CDialog::DoModal()
		{
			HWND hParent = m_parent;
			bool bIsParentEnable = false;
			if (hParent)
			{
				if (::IsWindowEnabled(hParent))
				{
					::EnableWindow(hParent, FALSE);
					::EnableWindow(m_hWnd, TRUE);
					bIsParentEnable = true;
				}
			}
			
			m_looping = true;
			int result = messageloop::RunModalLoop(m_hWnd, hParent, NULL);
			m_looping = false;

			if (bIsParentEnable)
			{
				::EnableWindow(hParent, TRUE);
				::SetActiveWindow(hParent);
			}

			::SetForegroundWindow(hParent);
			::SetFocus(hParent);

			return result;
		}

		int CDialog::MessageLoop()
		{
			m_looping = true;
			int result = ::view::MessageLoop();
			m_looping = false;
			return result;
		}

		void CDialog::ShowText(unsigned short id, bool show_)
		{
			CStaticText::ShowText(id, show_);

			RedrawText();
		}

		void CDialog::SwitchText(unsigned short id)
		{
			CStaticText::SwitchText(id);

			RedrawText();
		}

		void CDialog::CreateMemoryDC()
		{
			SkResize(m_width, m_height);
			RedrawBackground();
		}

		void CDialog::RedrawText()
		{
			RedrawBackground();
			Redraw();
		}

		void CDialog::RedrawBackground()
		{
			SkCanvas canvas(m_skbitmap);

			//dialog Background
			skin::CSkinData::OnDrawDialogBackground(canvas, this, m_width, m_height);

			//»­Static Text
			OnDrawText(canvas, SK_ColorBLACK);

			SendRedrawBackgroundMessage(m_hWnd);
		}

		void CDialog::SendRedrawBackgroundMessage(HWND hWnd)
		{
			::SendMessage(hWnd, WM_TA_REDRAW_BACKGROUND, NULL, NULL);

			HWND hwndChild = ::GetWindow(hWnd, GW_CHILD);  
			while(hwndChild) {   
				SendRedrawBackgroundMessage(hwndChild);
				hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);   
			}
		}

		void CDialog::Redraw()
		{
			RECT client = {0};
			::GetClientRect(m_hWnd, &client);
			::InvalidateRect(m_hWnd, &client, TRUE);
		}

		bool CDialog::CreateDialogEx(HWND hParent, LPCWSTR lpszTitle, DWORD dwStyle, bool bIsModal, bool bHide, DialogShowPosition pos)
		{
			int cx = GetSystemMetrics(SM_CXSCREEN);
			int cy = GetSystemMetrics(SM_CYSCREEN);

			int left = 0;
			int top = 0;

			switch (pos) {
			case kDSPRightTop:
				left = cx - GetInitialWidth() - 100;
				top = 100;
				break;
			case kDSPCenter:
			default:
				left = (cx - GetInitialWidth()) / 2;
				top = (cy - GetInitialHeight()) / 2;
				break;
			}

			m_parent = hParent;
			if (CreateEx(hParent, kDialogClassName, lpszTitle,
							 dwStyle,
							 WS_EX_CONTROLPARENT,
				left, top,
				GetInitialWidth(), GetInitialHeight()))
			{
				if (bHide) {
					m_width = GetInitialWidth();
					m_height = GetInitialHeight();

					CreateMemoryDC();
				}

				Show(!bHide);
				return true;
			} else {
				return false;
			}

			return true;
		}

		bool CDialog::CreateDialog(HWND hParent, LPCWSTR lpszTitle, bool bIsModal, bool bHide, DialogShowPosition pos)
		{
			return CreateDialogEx(hParent, lpszTitle, 
							 WS_OVERLAPPED     | \
                             WS_CAPTION        | \
                             WS_SYSMENU        | \
                             WS_DLGFRAME     | \
                             (bIsModal ? DS_NOIDLEMSG : WS_MINIMIZEBOX),
						bIsModal, bHide, pos);
		}

		LRESULT CALLBACK CDialog::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
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
			case WM_SIZE:
				{
					if (!IsMinimized()) {
						handled = true;
						m_width = LOWORD(lParam); // width of client area
						m_height = HIWORD(lParam);
						
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
					if (::GetClientRect(hChildWnd, &rc) && ::GetClientRect(m_hWnd, &rcParent))
					{
						::ClientToScreen(hChildWnd, (LPPOINT)&rc);
						::ClientToScreen(hChildWnd, &(((LPPOINT)&rc)[1]));
						::ClientToScreen(m_hWnd, (LPPOINT)&rcParent);
						::ClientToScreen(m_hWnd, &(((LPPOINT)&rcParent)[1]));

						handled = true;

						//::SetTextColor(hdc, skin::CSkinData::m_si.color.ToCOLORREF());

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
					//::BitBlt(hdc, 0, 0, m_width, m_height, m_hMemDC, 0, 0, SRCCOPY);
					doSkPaint(hdc, 0, 0, m_width, m_height, 0, 0);
				}
				break;
			case WM_PAINT:
				{
					handled = true;
					PAINTSTRUCT ps = {0};
					HDC hdc = BeginPaint(m_hWnd, &ps);
					//::BitBlt(hdc, 0, 0, m_width, m_height, m_hMemDC, 0, 0, SRCCOPY);
					doSkPaint(hdc, 0, 0, m_width, m_height, 0, 0);
					EndPaint(m_hWnd, &ps);
				}
				break;
			case WM_GETICON:
				handled = true;
				switch (wParam)
				{
				case ICON_SMALL:
					return (LRESULT)m_small_icon;
				case ICON_BIG:
					return (LRESULT)m_big_icon;
				}
				break;
			}
			return 0;
		}

		void CDialog::OnSize(int width, int height)
		{
		}
	};
};
