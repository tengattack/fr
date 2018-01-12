
#ifndef _SNOW_CUTE_FRAME_WINDOW_H_
#define _SNOW_CUTE_FRAME_WINDOW_H_ 1

#include "../common.h"

#ifdef IsMaximized
#undef IsMaximized
#endif

#ifdef IsMinimized
#undef IsMinimized
#endif

namespace view{
	namespace frame{

		enum WindowType{
			kWTUnknow = 0,
			kWTDialog,
			kWTMain,
			kWTView,
			kWTButton,
			kWTTitleTab,
			kWTPBar,
			kWTLink
		};
		
		class CWindow{
		public:
			CWindow();
			virtual ~CWindow();

			bool Create(HWND hParent, LPCWSTR lpszClass, LPCWSTR lpszTitle, DWORD dwStyle, int x, int y, int width, int height, HMENU hMenu = NULL);
			bool CreateEx(HWND hParent, LPCWSTR lpszClass, LPCWSTR lpszTitle, DWORD dwStyle, DWORD dwExStyle, int x, int y, int width, int height, HMENU hMenu = NULL);

			void SetText(LPCWSTR lpszText);
			int GetText(std::wstring& str);
			int GetTextLength();

			void Enable(bool bEnable = true);
			void Show(bool bShow = true);
			void Close();

			bool IsVisible();
			bool IsEnabled();

			bool IsMaximized();
			bool IsMinimized();

			HWND SetFocus();

			HWND hWnd();
			HWND GetParent();

			bool SetWindowPos(HWND hWndInsertAfter, int x, int y, int width, int height, DWORD flags = SWP_NOACTIVATE | SWP_NOCOPYBITS);

			bool Move(int x, int y);
			bool Size(int width, int height);

			//function
			int MsgBoxInfo(LPCWSTR lpszText);
			int MsgBoxError(LPCWSTR lpszText);

			static bool OnPaintParentBackground(HWND hWnd, HDC hdc);

		//protected:
			//message
			virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
			virtual WindowType GetType();

		protected:
			//var
			HWND m_hWnd;
			bool m_bCreate;

			virtual bool CreateImpl();
		};
	};
};

#endif