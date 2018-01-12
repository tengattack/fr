

#ifndef _SNOW_CUTE_TEXT_H_
#define _SNOW_CUTE_TEXT_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"

namespace view{
	namespace frame{
		
		class CText : public CChild{
		public:
			CText();
			~CText();

			bool CreateText(HWND hParent, LPCWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL);

			void ReplaceSel(LPCWSTR lpszText, bool undone_ = true);

		//protected:
			//virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			/*virtual int OnMouseMove(bool movein);
			virtual int OnMouseLeave();*/
			
		protected:
			bool m_bHover;
		};
	};
};

#endif