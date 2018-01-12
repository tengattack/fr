

#ifndef _SNOW_CUTE_RICHTEXT_H_
#define _SNOW_CUTE_RICHTEXT_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "text.h"
#include <base/lock.h>
#include <richedit.h>

namespace view{
	namespace frame{
		enum RichTextVersion {
			rt20 = 0,
			rt41,
			rtError
		};

		struct LibraryAndClassName {
			wchar_t* libname;
			wchar_t* classname;
		};

		static LibraryAndClassName szRichTextLACN[] = {
			{ L"riched20.dll", RICHEDIT_CLASSW },
			{ L"msftedit.dll", MSFTEDIT_CLASS },
			{ NULL, NULL }
		};

		class CRichText : public CText{

		public:
			CRichText();
			~CRichText();

			static bool InitRichText();
			static void UninitRichText();
			static RichTextVersion m_rtv;
			static DWORD CALLBACK StreamInCallBack(DWORD dwCookie, LPBYTE pbBuff, LONG cb,
						LONG *pcb);
			static DWORD CALLBACK StreamOutCallBack(DWORD dwCookie, LPBYTE pbBuff, LONG cb,
						LONG *pcb);

			bool CreateRichText(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP | WS_VSCROLL |
					ES_AUTOVSCROLL | ES_WANTRETURN | ES_MULTILINE);

			void SetOptions(WORD wOp, DWORD dwFlags);
			
			void GetText(std::wstring& text, int nFormatEx = NULL);
			void SetText(LPCSTR text, int nFormatEx = NULL);

			void GetRTF(std::string& rfttext, int nFormatEx = NULL);	//SFF_SELECTION
			void SetRTF(LPCSTR rfttext, int nFormatEx = SFF_SELECTION);	//默认设置选中的文本

			int SetSelEx(int cpMin, int cpMax);
			int GetSelEx(CHARRANGE& charrange);

			DWORD SetEditStyle(DWORD dwStyle);
			DWORD GetEditStyle();

			DWORD SetEventMask(DWORD mask);
			DWORD GetEventMask();

			//virtual void LockEvent();
			//virtual void UnlockEvent();

		protected:
			Lock m_lock;
			//bool m_lockevent;

			static HMODULE m_hRichTextDll;

			void GetStream(int nFormat, std::string& stream);
			void GetStream(int nFormat, std::wstring& stream);
			void SetStream(int nFormat, std::string& stream);
			//void SetStream(int nFormat, std::wstring& stream);

			int StreamOut(int nFormat, EDITSTREAM& es);
			int StreamIn(int nFormat, EDITSTREAM& es);

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
		};
	};
};

#endif