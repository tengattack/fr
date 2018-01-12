

#include "frame.h"
#include "frame_window.h"
#include "frame_child.h"
#include "messageloop.h"

#include "../global.h"
#include "../view.h"

#include <vector>

#define TA_MESSAGE_THREAD		(WM_USER + 0x10)

#ifndef WM_KICKIDLE
#define WM_KICKIDLE 0x036A
#endif

namespace view{
	namespace frame{
		namespace messageloop{
			
			int m_loop_thread_count = 0;
			std::vector<DWORD> m_vecThreadId;

			DWORD WINAPI tLoopProc(LPVOID lParam)
			{
				MSG msg = {0};

				//to build queue
				::DefWindowProcW(NULL, 0, 0, 0);
				PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

				while (GetMessage(&msg, NULL, 0, 0))
				{
					//if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
					if (msg.message == TA_MESSAGE_THREAD)
					{
						TranslateMessage((MSG *)msg.wParam);
						DispatchMessage((MSG *)msg.wParam);

						free((MSG *)msg.wParam);
					}
				}

				return 0;//(int)msg.wParam;
			}

			int InitLoopThread(int count)
			{
				HANDLE hThread = NULL;
				DWORD dwThreadId = 0;
				for (int i = 0; i < count; i++)
				{
					//创建消息循环线程
					hThread = ::CreateThread(NULL, 0, tLoopProc, NULL, NULL, &dwThreadId);
					if (hThread)
					{
						m_vecThreadId.push_back(dwThreadId);
						CloseHandle(hThread);
					}
				}
				m_loop_thread_count = (int)m_vecThreadId.size();
				return m_loop_thread_count;
			}

			bool TranslateDialogMessage(MSG* pMsg)
			{
				//主要为tab按键切换控件
				HWND hParent = GetParent(pMsg->hwnd);
				return IsDialogMessage(hParent, pMsg);
			}

			bool IsForwardMessage(MSG* pMsg)
			{
				CWindow *pWindow = GetWindow(pMsg->hwnd);
				if (pWindow)
				{
					WindowType type = pWindow->GetType();
					if (type == kWTMain || type== kWTView
							|| type == kWTButton || type == kWTTitleTab || type == kWTLink)
					{
						switch (pMsg->message)
						{
						case WM_TIMER:
						//case WM_NCHITTEST:
						//case WM_NCPAINT:
							return true;
						}
					}
				}
				return false;
			}

			int Loop()
			{
				// 消息循环:
				MSG msg = {0};
				int iThread = 0;
				while (GetMessage(&msg, NULL, 0, 0))
				{
					
					//if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
					if (IsForwardMessage(&msg))
					{
						//转发消息
						MSG *pMsg = (MSG *)malloc(sizeof(MSG));
						if (pMsg)
						{
							memcpy(pMsg, &msg, sizeof(MSG));
							PostThreadMessage(m_vecThreadId[iThread++], TA_MESSAGE_THREAD, (WPARAM)pMsg, NULL);
							if (iThread >= m_loop_thread_count)
							{
								iThread = 0;
							}
						}
					} else {
						if (!TranslateDialogMessage(&msg)) {
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
					}
				}

				return (int)msg.wParam;
			}

			int RunModalLoop(HWND hWnd, HWND hWndParent, DWORD dwFlags)
			{
				//ASSERT(::IsWindow(m_hWnd)); // window must be created
				//ASSERT(!(m_nFlags & WF_MODALLOOP)); // window must not already be in modal state
 
				// for tracking the idle time state
				bool bIdle = true;
				bool bShowIdle = true;
				LONG lIdleCount = 0;
			
				MSG m_msgCur = {0};
				MSG* pMsg = &m_msgCur;
				int iThread = 0;

				// acquire and dispatch messages until the modal state is done
				for (;;)
				{
					//ASSERT(ContinueModal());
 
					// phase1: check to see if we can do idle work
					// 第一个循环：处理空闲工作，更新界面，状态栏，工具栏，发生WM_KICKIDLE消息，用于扩充处理。
					while (bIdle &&
						!::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE))
					{
						//ASSERT(ContinueModal());
 
						// call OnIdle while in bIdle state
						if (/*!(dwFlags & MLF_NOIDLEMSG) && hWndParent != NULL &&*/ lIdleCount == 0)
						{
							// send WM_ENTERIDLE to the parent
							::SendMessage(hWndParent, WM_ENTERIDLE, MSGF_DIALOGBOX, (LPARAM)hWnd);
						}
						if (/*(dwFlags & MLF_NOKICKIDLE) ||*/
							!SendMessage(hWnd, WM_KICKIDLE, MSGF_DIALOGBOX, lIdleCount++))
						{
							// stop idle processing next time
							bIdle = false;
						}
					}
 
					// phase2: pump messages while available
					do {
						// ASSERT(ContinueModal());
 
						// pump message, but quit on WM_QUIT
						// 下面专门分析PumpMessage
						if (!PumpMessage(pMsg))
						{
							//PostQuitMessage(0);
							goto exit_modal;
						}
 
						// show the window when certain special messages rec'd
						if (bShowIdle &&
							(pMsg->message == 0x118 || pMsg->message == WM_SYSKEYDOWN))
						{
							ShowWindow(hWnd, SW_SHOWNORMAL);
							UpdateWindow(hWnd);
							bShowIdle = true;
						}

						// reset "no idle" state after pumping "normal" message
						if (IsIdleMessage(pMsg))
						{
							bIdle = true;
							lIdleCount = 0;
						}

						if (IsForwardMessage(pMsg))
						{
							//转发消息
							MSG *pMsg2 = (MSG *)malloc(sizeof(MSG));
							if (pMsg2)
							{
								memcpy(pMsg2, pMsg, sizeof(MSG));
								PostThreadMessage(m_vecThreadId[iThread++], TA_MESSAGE_THREAD, (WPARAM)pMsg2, NULL);
								if (iThread >= m_loop_thread_count)
								{
									iThread = 0;
								}
							}
						}

					} while (::PeekMessage(pMsg, NULL, NULL, NULL, PM_NOREMOVE));
				}

			exit_modal:
				return (int)pMsg->wParam;
			}

			bool IsIdleMessage(MSG* pMsg)
			{
				//To improve such an application’s performance, override IsIdleMessage in the application’s CWinApp-derived class to check for WM_TIMER messages as follows:
				if (pMsg->message == WM_TIMER)
					return false;

				// Return FALSE if the message just dispatched should _not_
				// cause OnIdle to be run.  Messages which do not usually
				// affect the state of the user interface and happen very
				// often are checked for.
				static POINT m_ptCursorLast = {0};
				static UINT m_nMsgLast = 0;

				// redundant WM_MOUSEMOVE and WM_NCMOUSEMOVE
				if (pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE)
				{
					// mouse move at same position as last mouse move?
					if (memcmp(&m_ptCursorLast, &pMsg->pt, sizeof(POINT)) == 0 
						&& pMsg->message == m_nMsgLast)
						return false;

					m_ptCursorLast = pMsg->pt;  // remember for next time
					m_nMsgLast = pMsg->message;
					return true;
				}

				// WM_PAINT and WM_SYSTIMER (caret blink)
				return pMsg->message != WM_PAINT && pMsg->message != 0x0118;
			}

			bool PumpMessage(MSG* pMsg)
			{
				if (!::GetMessage(pMsg, NULL, NULL, NULL))
				{
					return false;
				}

				static int pump_iThread = 0;
				// process this message
				if (pMsg->message != WM_KICKIDLE)
				{
					if (IsForwardMessage(pMsg))
					{
						//转发消息
						/*MSG *pMsg = (MSG *)malloc(sizeof(MSG));
						memcpy(pMsg, pMsg, sizeof(MSG));
						PostThreadMessage(m_vecThreadId[pump_iThread++], TA_MESSAGE_THREAD, (WPARAM)pMsg, NULL);
						if (pump_iThread >= m_loop_thread_count)
						{
							pump_iThread = 0;
						}*/
					} else {
						if (!TranslateDialogMessage(pMsg)) {
							TranslateMessage(pMsg);
							DispatchMessage(pMsg);
						}
					}
				}
				return true;
			}

		};
	};
};