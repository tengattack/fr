
#ifndef _SNOW_CUTE_MAIN_WINDOW_H_
#define _SNOW_CUTE_MAIN_WINDOW_H_ 1

#include "../common.h"
#include "dialog.h"
#include "skin.h"
#include <vector>

namespace view{
	namespace frame{
	
		static const wchar_t kMainWindowClassName[] = L"taSnowCute";	// 主窗口类名

		class CMainWindow : public CDialog{

		public:
			CMainWindow();
			~CMainWindow();

			static void InitMainWindow();

			bool CreateMainWindow(LPCWSTR lpszTitle, bool show = true);
		
			//virtual void InitWindow();
			virtual void CreateMemoryDC();
			virtual void RedrawBackground();

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
			virtual WindowType GetType();
			virtual int AddStaticText(unsigned short id, int x, int y, int width, int height, LPCWSTR lpszText, bool show_ = true, int text_size = -1);

		protected:
			skin::CSkin m_skin;
		};
	};
};

#endif