
#ifndef _SNOW_CUTE_FRAME_VIEW_H_
#define _SNOW_CUTE_FRAME_VIEW_H_ 1

#include "../common.h"
#include "static_text.h"
#include "frame_custom.h"
#include "frame_skia.h"
#include "view_container.h"
#include <vector>

namespace view{
	namespace frame{
		
		static const wchar_t kViewClassName[] = L"taView";

		class CView : public CCustom, public CStaticText{
		public:
			friend class CViewContainer;

			CView();
			virtual ~CView();

			virtual void ShowView(bool show_ = true);

			static bool RegisterClass();

			bool CreateView(HWND hParent, LPCWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP);
			int CreateView(CViewContainer *pContainer, LPCWSTR lpszText, unsigned short id, DWORD dwStyle = WS_CHILD | WS_TABSTOP);

			void AddWindow(CWindow *pWindow);

			void SetTextColor(SkColor color);

		//protected:
			//message
			//virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
			virtual WindowType GetType();

			virtual void UIUpdate(bool force_ = false);
			virtual void OnDraw(SkCanvas& canvas);
			virtual void Redraw(RECT *rect = NULL);

			virtual int AddStaticText(unsigned short id, int x, int y, int width, int height, LPCWSTR lpszText, bool show_ = true, int text_size = -1);
			virtual void SetStaticText(int index, LPCWSTR lpszText, bool show_ = true);

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

		protected:
			
			SkColor m_text_color;

			std::vector<CWindow *> m_window_list;
			
		};
	};
};

#endif