
#include "frame.h"
#include "frame_window.h"

#include "../global.h"
#include "../view.h"

namespace view{
	namespace frame{
		CWindow::CWindow()
			: m_hWnd(NULL)
			, m_bCreate(false)
		{
		}
		CWindow::~CWindow()
		{
			if (m_bCreate)
				DeleteWindow(this);
		}

		WindowType CWindow::GetType()
		{
			return kWTUnknow;
		}

		bool CWindow::CreateEx(HWND hParent, LPCWSTR lpszClass, LPCWSTR lpszTitle, DWORD dwStyle, DWORD dwExStyle, int x, int y, int width, int height, HMENU hMenu)
		{
			HWND hWnd = CreateWindowEx(dwExStyle, lpszClass, lpszTitle, dwStyle,
				x, y, width, height, hParent, hMenu, global::hInstance, NULL);

			if (!hWnd)
			{
				return false;
			}

			m_bCreate = true;
			m_hWnd = hWnd;

			return CreateImpl();
		}

		bool CWindow::Create(HWND hParent, LPCWSTR lpszClass, LPCWSTR lpszTitle, DWORD dwStyle, int x, int y, int width, int height, HMENU hMenu)
		{
			HWND hWnd = CreateWindow(lpszClass, lpszTitle, dwStyle,
				x, y, width, height, hParent, hMenu, global::hInstance, NULL);

			if (!hWnd)
			{
				return false;
			}

			m_bCreate = true;
			m_hWnd = hWnd;

			return CreateImpl();
		}

		bool CWindow::CreateImpl()
		{
			InsertWindow(m_hWnd, this);
			SendMessage(m_hWnd, WM_SETFONT, (WPARAM)global::hFont, MAKELPARAM(TRUE, 0));
			InitWindow();

			return true;
		}
		//function

		void CWindow::SetText(LPCWSTR lpszText)
		{
			SetWindowTextW(m_hWnd, lpszText);
		}

		int CWindow::GetTextLength()
		{
			return ::GetWindowTextLengthW(m_hWnd);
		}

		int CWindow::GetText(std::wstring& str)
		{
			int textlen = ::GetWindowTextLengthW(m_hWnd);
			if (textlen > 0)
			{
				str.resize(textlen);	//最后一位系统设置为0
				GetWindowTextW(m_hWnd, (LPWSTR)str.data(), textlen + 1);
			}
			return textlen;
		}

		void CWindow::Enable(bool bEnable)
		{
			EnableWindow(m_hWnd, bEnable ? TRUE : FALSE);
		}

		void CWindow::Show(bool bShow)
		{
			if (m_hWnd)
			{
				if (bShow)
				{
					ShowWindow(m_hWnd, SW_SHOW);
					UpdateWindow(m_hWnd);
				} else {
					ShowWindow(m_hWnd, SW_HIDE);
				}
			}
		}

		void CWindow::Close()
		{
			SendMessage(m_hWnd, WM_CLOSE, NULL, NULL);
		}

		bool CWindow::IsVisible()
		{
			return static_cast<bool>(::IsWindowVisible(m_hWnd));
		}

		bool CWindow::IsEnabled()
		{
			return static_cast<bool>(::IsWindowEnabled(m_hWnd));
		}

		bool CWindow::IsMaximized()
		{
			return static_cast<bool>(::IsZoomed(m_hWnd));
		}

		bool CWindow::IsMinimized()
		{
			return static_cast<bool>(::IsIconic(m_hWnd));
		}

		HWND CWindow::SetFocus()
		{
			return ::SetFocus(m_hWnd);
		}

		int CWindow::MsgBoxInfo(LPCWSTR lpszText)
		{
			return MessageBox(m_hWnd, lpszText, L"提示", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
		}

		int CWindow::MsgBoxError(LPCWSTR lpszText)
		{
			return MessageBox(m_hWnd, lpszText, L"错误", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
		}

		HWND CWindow::hWnd()
		{
			return m_hWnd;
		}

		HWND CWindow::GetParent()
		{
			return ::GetParent(m_hWnd);
		}

		bool CWindow::SetWindowPos(HWND hWndInsertAfter, int x, int y, int width, int height, DWORD flags)
		{
			return static_cast<bool>(::SetWindowPos(m_hWnd, hWndInsertAfter, x, y, width, height, flags));
		}

		bool CWindow::Move(int x, int y)
		{
			return SetWindowPos(NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
		}

		bool CWindow::Size(int width, int height)
		{
			return SetWindowPos(NULL, 0, 0, width, height, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
		}

		void CWindow::InitWindow()
		{
		}

		LRESULT CALLBACK CWindow::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_CLOSE:
				m_bCreate = false;
				break;
			}
			//handled = false;
			return 0;
		}

		bool CWindow::OnPaintParentBackground(HWND hWnd, HDC hdc)
		{
			return SkiaPaintParent(hWnd, hdc, true);
		}
	};
};
