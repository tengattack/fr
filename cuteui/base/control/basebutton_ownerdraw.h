

#ifndef _SNOW_CUTE_BASE_BUTTON_OWNERDRAW_H_
#define _SNOW_CUTE_BASE_BUTTON_OWNERDRAW_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "../frame/frame_skia.h"
#include "basebutton.h"

namespace view {
	namespace frame {

		class CBaseButtonOwnerDraw : public CBaseButton, public SkiaInterface {
		public:

			/*enum BUTTONPARTS {
				BP_PUSHBUTTON = 1,
				BP_RADIOBUTTON = 2,
				BP_CHECKBOX = 3,
				BP_GROUPBOX = 4,
				BP_USERBUTTON = 5,
				BP_COMMANDLINK = 6,
				BP_COMMANDLINKGLYPH = 7,
			};*/
			
			enum ButtonOwnerDrawStyle {
				kStylePush = 1,
				kStyleRadio,
				kStyleCheckbox,
				kStyleGroupbox,
			};

			CBaseButtonOwnerDraw(ButtonOwnerDrawStyle style);
			~CBaseButtonOwnerDraw();

			static const int kFadeStep = 55;
			static const int kFadeElapse = 60;

			bool CreateButtonOwnerDraw(HWND hParent, LPWSTR lpszText, unsigned short id, 
					int x, int y, int width, int height, DWORD dwStyle);

			void SetTextColor(SkColor color);

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
			virtual WindowType GetType();

		protected:

			ButtonOwnerDrawStyle m_style;

			int m_control_alpha;
			int m_width, m_height;

			SkColor m_text_color;

			virtual void CreateMemoryDC();

			virtual void OnDrawText(HDC hdc);
			virtual void OnDrawControl(HDC hdc, BYTE alpha = 255);
			virtual void OnPaint(HDC hdc);

		};
	};
};

#endif