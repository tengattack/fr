
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"

#include "movablewindow.h"

namespace view {
	namespace frame {

		MovableWindow::MovableWindow()
			: m_last_get_pt(false)
		{
			memset(&m_last_cursor_pt, 0, sizeof(POINT));
		}

		bool MovableWindow::CheckMoving(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			bool moving_ = false;
			switch (message)
			{
			case WM_NCMOUSEMOVE:
			case WM_MOUSEMOVE:
				if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
					moving_ = true;
					break;
				}
				if (!m_last_get_pt) {
					if (GetCursorPos(&m_last_cursor_pt)) {
						m_last_get_pt = true;
					}
					moving_ = true;
					break;
				}
			case WM_NCLBUTTONUP:
			case WM_LBUTTONUP:
				moving_ = true;
				if (m_last_get_pt) {
					POINT pt_move;
					RECT rc;
					if (GetCursorPos(&pt_move) && GetWindowRect(hWnd, &rc)) {
						::SetWindowPos(hWnd, NULL, 
							rc.left + pt_move.x - m_last_cursor_pt.x,
							rc.top + pt_move.y - m_last_cursor_pt.y,
							0, 0, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOSIZE);

						if (message == WM_LBUTTONUP || message == WM_NCLBUTTONUP) {
							m_last_get_pt = false;
						} else {
							memcpy(&m_last_cursor_pt, &pt_move, sizeof(POINT));
						}
					}
				}
				break;
			case WM_NCLBUTTONDOWN:
			case WM_LBUTTONDOWN:
				moving_ = true;
				//允许点击窗口任意位置拖动 (将点击转到非客户区)
				//::PostMessage(m_hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
				GetCursorPos(&m_last_cursor_pt);
				m_last_get_pt = true;
				break;
			}
			return moving_;
		}
	}
}