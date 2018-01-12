
#ifndef _SNOW_CUTE_CAPTIONBUTTON_H_
#define _SNOW_CUTE_CAPTIONBUTTON_H_ 1

#include "frame_window.h"

#include "../common.h"

namespace view{
	namespace frame{
		enum CAPTION_BUTTON_TYPE{
			kCBClose = 0,
			kCBMax,
			kCBMin,
			kCBUnknow
		};

		class CCaptionButton{
		public:
			CCaptionButton();
			~CCaptionButton();

			BYTE GetHitTest();
			void SetHitTest(BYTE hittest);

			void SetBounds(RECT *pRect);
			void GetBounds(RECT *pRect);

			inline bool GetPressed()
			{
				return _pressed;
			}

			inline void SetPressed(bool pressed)
			{
				if (_pressed && !pressed)
					m_just_pressed = true;
				else
					m_just_pressed = false;

				_pressed = pressed;
				
			}

			inline bool GetHovered()
			{
				return _hovered;
			}

			inline void SetHovered(bool hovered)
			{
				if (_hovered && !hovered)
				{
					m_just_hovered = true;
					m_just_pressed = false;
				}
				else
				{
					m_just_hovered = false;
				}

				_hovered = hovered;
				//if (!hovered) m_just_pressed = false;
			}

			inline bool GetEnabled()
			{
				return _enabled;
			}

			inline void SetEnabled(bool enabled)
			{
				_enabled = enabled;
			}

			inline bool GetShow()
			{
				return _show;
			}

			inline void SetShow(bool show)
			{
				_show = show;
			}

			inline CAPTION_BUTTON_TYPE GetType()
			{
				switch (_hittest)
				{
				case HTCLOSE:
					return kCBClose;
				case HTMAXBUTTON:
					return kCBMax;
				case HTMINBUTTON:
					return kCBMin;
				default:
					return kCBUnknow;
				}
			}

			int m_fade;
			bool m_just_pressed;
			bool m_just_hovered;

	protected:
			RECT _bounds;
			bool _show;
			bool _pressed;
			bool _hovered;
			bool _enabled;
			BYTE _hittest;
			
		};
	};
};

#endif