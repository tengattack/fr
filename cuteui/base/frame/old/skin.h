
#ifndef _SNOW_CUTE_SKIN_H_
#define _SNOW_CUTE_SKIN_H_ 1

#include "frame_window.h"

#include "../common.h"
#include "captionbutton.h"

namespace view{
	namespace frame{
		namespace skin{
			static const int MAX_CAPTION_BUTTON = 3;

			class CSkin{
			public:
				CSkin(CWindow *pWindow);
				~CSkin();

				LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
				bool OnNcPaint(bool invalidateBuffer);
				
				CCaptionButton* CommandButtonFromPoint(LPPOINT lpPoint);
				CCaptionButton* CommandButtonByHitTest(BYTE hittest);
				void UpdateCaption();

				int OnNcHitTest(POINTS* lpPoint, bool& handled);
				void OnNcLButtonUp(BYTE hittest, bool& handled);
				void OnNcLButtonDown(BYTE hittest, bool& handled);
				void OnNcMouseMove(BYTE hittest, bool& handled);
				void OnSize(WPARAM wParam, LPARAM lParam);
				void OnTimer(int nIDEvent, bool& handled);

				void OnCaptionButtonPaint(CCaptionButton* button, HDC hCaptionDC = NULL);

				void UpdateStyle();
				
				CCaptionButton* GetCaptionButton(int index);
			
			protected:

				CCaptionButton *_pressedButton, *_hoveredButton;
				CCaptionButton m_caption_button[MAX_CAPTION_BUTTON];

				void UpdateSystemMenu(HMENU hMenu);

				CWindow *m_pWindow;
				bool _formIsActive;
				bool m_bHover;
			};
		};
	};
};

#endif