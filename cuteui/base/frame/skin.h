
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

				void OnCaptionButtonPaint(CCaptionButton* button, SkCanvas *pCanvas = NULL);

				void UpdateStyle();
				
				CCaptionButton* GetCaptionButton(int index);

				inline void SetTitleTextSize(int text_size) {
					m_title_text_size = text_size;
				}

				inline void SetDrawIcon(bool draw_ = true) {
					if (draw_ != m_draw_icon) {
						m_draw_icon = draw_;
					}
				}

				inline void SetIconDstRect(SkRect& rect) {
					m_icon_dst_rect = rect;
				}

				void SetIcon(SkBitmap *icon_bitmap, SkBitmap::Config config = SkBitmap::kARGB_8888_Config);
			
			protected:

				CCaptionButton *_pressedButton, *_hoveredButton;
				CCaptionButton m_caption_button[MAX_CAPTION_BUTTON];

				void UpdateSystemMenu(HMENU hMenu);

				CWindow *m_pWindow;
				bool _formIsActive;
				bool m_bHover;

				int m_title_text_size;
				bool m_draw_icon;
				SkBitmap *m_icon_bitmap;
				SkRect m_icon_dst_rect;	// def: {4, 4, 20, 20}
			};
		};
	};
};

#endif