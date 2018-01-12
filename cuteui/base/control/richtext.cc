
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "richtext.h"

#include "base/view_gdiplus.h"

//#include <Richole.h>

namespace view{
	namespace frame{

		struct REQSIZE 
		{
			NMHDR nmhdr; 
			RECT rect; 
		};

		RichTextVersion CRichText::m_rtv = rtError;
		HMODULE CRichText::m_hRichTextDll = NULL;

		struct STREAM_JOB_INFO {
			CRichText *rt;
			void* pointer;
			int type;	//0 ascii 1 unicode
			int nextsize;
			//HANDLE event;
		};

		CRichText::CRichText()
			//: m_lockevent(false)
		{
		}
		CRichText::~CRichText()
		{
		}

		int CRichText::StreamOut(int nFormat, EDITSTREAM& es)
		{
			return ::SendMessage(m_hWnd, EM_STREAMOUT, (WPARAM)nFormat, (LPARAM)&es);
		}

		int CRichText::StreamIn(int nFormat, EDITSTREAM& es)
		{
			return ::SendMessage(m_hWnd, EM_STREAMIN, (WPARAM)nFormat, (LPARAM)&es);
		}

		int CRichText::SetSelEx(int cpMin, int cpMax)
		{
			CHARRANGE cr = {cpMin, cpMax};
			return ::SendMessage(m_hWnd, EM_EXSETSEL, NULL, (LPARAM)&cr);
		}

		int CRichText::GetSelEx(CHARRANGE& charrange)
		{
			return ::SendMessage(m_hWnd, EM_EXGETSEL, NULL, (LPARAM)&charrange);
		}

		DWORD CALLBACK CRichText::StreamOutCallBack(DWORD dwCookie, LPBYTE pbBuff, LONG cb,
           LONG *pcb)
		{
			STREAM_JOB_INFO *pInfo = (STREAM_JOB_INFO *)dwCookie;
			if (!pInfo) return 0;

			switch (pInfo->type)
			{
			case 0:
				*(std::string *)(pInfo->pointer) += std::string((char *)pbBuff, cb);
				break;
			case 1:
				*(std::wstring *)(pInfo->pointer) += std::wstring((wchar_t *)pbBuff, cb / sizeof(wchar_t));
				break;
			default:
				return 1;
			}
			
			return 0;
		}


		DWORD CALLBACK CRichText::StreamInCallBack(DWORD dwCookie, LPBYTE pbBuff, LONG cb,
           LONG *pcb)
		{
			STREAM_JOB_INFO *pInfo = (STREAM_JOB_INFO *)dwCookie;
			if (!pInfo) return 0;


			int len = 0;
			
			switch (pInfo->type)
			{
			case 0:
				len = ((std::string *)pInfo->pointer)->length();
				break;
			/*case 1:
				len = ((std::wstring *)pInfo->pointer)->length() * sizeof(wchar_t);
				break;*/
			default:
				return 1;
			}
			

			if (len > cb - 1)
			{
				len = cb - 1;
			}

			switch (pInfo->type)
			{
			case 0:
				memcpy(pbBuff, ((std::string *)pInfo->pointer)->c_str() + pInfo->nextsize, len);
				pInfo->nextsize += len;
				pbBuff[len] = 0;
				break;
			/*case 1:
				::WideCharToMultiByte(CP_ACP, 0, ((std::wstring *)pInfo->pointer)->c_str() + pInfo->nextsize, -1, (LPSTR)pbBuff, cb, NULL, NULL);
				pInfo->nextsize += len;
				pbBuff[len] = 0;
				break;*/
			default:
				return 1;
			}

			*pcb = len;

			//LPCWSTR lpszText = (LPCWSTR)dwCookie;
			//WideCharToMultiByte(CP_OEMCP, NULL, lpszText, -1, (LPSTR)pbBuff, cb, NULL, FALSE); //这个很重要，千万要转换为单字节
			return 0;
		}

		void CRichText::SetRTF(LPCSTR rfttext, int nFormatEx)
		{
			std::string str = rfttext;
			SetStream(SF_RTF | nFormatEx, str);
		}

		void CRichText::GetRTF(std::string& rfttext, int nFormatEx)
		{
			GetStream(SF_RTF | nFormatEx, rfttext);
		}

		void CRichText::GetText(std::wstring& text, int nFormatEx)
		{
			GetStream(SF_TEXT | SF_UNICODE | nFormatEx, text);
		}

		void CRichText::SetText(LPCSTR text, int nFormatEx)
		{
			std::string str = text;
			SetStream(SF_TEXT | nFormatEx, str);
		}

		void CRichText::GetStream(int nFormat, std::wstring& stream)
		{
			AutoLock al(m_lock);

			//HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);

			STREAM_JOB_INFO sji = {this, &stream, 1, 0/*event*/};

			EDITSTREAM es = {(DWORD)&sji, 0, StreamOutCallBack};
			StreamOut(nFormat, es);

			//WaitForSingleObject(event, INFINITE);
			//CloseHandle(event);
		}

		void CRichText::GetStream(int nFormat, std::string& stream)
		{
			AutoLock al(m_lock);

			STREAM_JOB_INFO sji = {this, &stream, 0, 0};

			EDITSTREAM es = {(DWORD)&sji, 0, StreamOutCallBack};
			StreamOut(nFormat, es);
		}

		void CRichText::SetStream(int nFormat, std::string& stream)
		{
			AutoLock al(m_lock);

			STREAM_JOB_INFO sji = {this, &stream, 0, 0};

			EDITSTREAM es = {(DWORD)&sji, 0, StreamInCallBack};
			StreamIn(nFormat, es);
		}

		/*void CRichText::SetStream(int nFormat, std::wstring& stream)
		{
			AutoLock al(m_lock);

			STREAM_JOB_INFO sji = {this, &stream, 1, 0};

			EDITSTREAM es = {(DWORD)&sji, 0, StreamInCallBack};
			StreamIn(nFormat, es);
		}*/

		void CRichText::SetOptions(WORD wOp, DWORD dwFlags)
		{
			::SendMessage(m_hWnd, EM_SETOPTIONS, (WPARAM)wOp, (LPARAM)dwFlags);  
		}

		DWORD CRichText::SetEditStyle(DWORD dwStyle)
		{
			return SendMessage(m_hWnd, EM_SETEDITSTYLE, dwStyle, 0);
		}

		DWORD CRichText::GetEditStyle()
		{
			return SendMessage(m_hWnd, EM_GETEDITSTYLE, 0, 0);
		}

		DWORD CRichText::SetEventMask(DWORD mask)
		{
			return ::SendMessage(m_hWnd, EM_SETEVENTMASK, 0, mask);
		}

		DWORD CRichText::GetEventMask()
		{
			return ::SendMessage(m_hWnd, EM_GETEVENTMASK, 0, 0);
		}

		bool CRichText::InitRichText()
		{
			//向旧版本推
			if (m_hRichTextDll = LoadLibrary(szRichTextLACN[rt41].libname))
			{
				m_rtv = rt41;
				return true;
			} else if (m_hRichTextDll = LoadLibrary(szRichTextLACN[rt20].libname)) {
				m_rtv = rt20;
				return true;
			} else {
				return false;
			}
		}

		void CRichText::UninitRichText()
		{
			FreeLibrary(m_hRichTextDll);
		}

		bool CRichText::CreateRichText(HWND hParent, LPWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (m_rtv == rtError)
			{
				return false;
			}
			if (CreateEx(hParent, szRichTextLACN[m_rtv].classname, lpszText, dwStyle, WS_EX_TRANSPARENT,
				x, y,
				width, height, (HMENU)id))
			{
				SetID(id);
				SubClassWindow();

				//SetOptions(ECOOP_SET, ECO_NOHIDESEL	| ECO_READONLY | ECO_AUTOVSCROLL | ECO_AUTOWORDSELECTION);
				/*SendMessage(m_hWnd, EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE);
				//SendMessage(m_hWnd, EM_REQUESTRESIZE, 0, 0);*/
				/*IRichEditOle *pREO = NULL;
				SendMessage(m_hWnd, EM_GETOLEINTERFACE, 0, (LPARAM)&pREO);
				pREO->InsertObject();
				pREO->Release();*/

				Show();
				return true;
			} else {
				return false;
			}

			return true;
		}

		/*void CRichText::LockEvent()
		{
			m_lockevent = true;
		}

		void CRichText::UnlockEvent()
		{
			m_lockevent = false;
		}*/

		LRESULT CALLBACK CRichText::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_ERASEBKGND:
				{
					// fix XP 背景问题（现在不兼容XP的richtext背景问题了）
					//if (OnPaintParentBackground(m_hWnd, (HDC)wParam))
					{
						handled = true;
						return TRUE;
					}
				}
				break;
			/*case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONDBLCLK:
			case WM_RBUTTONUP:
				if (m_lockevent)
				{
					handled = true;
					return TRUE;
				}
				break;
			/*case WM_NOTIFY:
				if (lParam)
				{
					NMHDR *hdr = (NMHDR *)lParam;

					if (hdr->code == EN_SELCHANGE)
					{
						::OutputDebugStringW(L"EN_SELCHANGE\r\n");
						handled = true;
					}

					//无底的RichEdit
					if (hdr->code == EN_REQUESTRESIZE)
					{
						REQSIZE *reqs = (REQSIZE *)lParam;
						//....
						handled = true;
					}
				}
				break;*/
			}

			return CChild::OnWndProc(message, wParam, lParam, handled);
		}
	};
};