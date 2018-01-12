 
#include "frame.h"
#include "frame_window.h"

#include "../global.h"
#include "../view.h"

#include "skin.h"
#include "captionbutton.h"

namespace view{
	namespace frame{
		CCaptionButton::CCaptionButton()
			: _enabled(true)
			, _hovered(false)
			, _pressed(false)
			, _hittest(HTNOWHERE)
			, _show(true)
			, m_fade(255)
			, m_just_pressed(false)
			, m_just_hovered(false)
		{
			memset(&_bounds, 0, sizeof(RECT));
		}
		CCaptionButton::~CCaptionButton()
		{
		}

		BYTE CCaptionButton::GetHitTest()
		{
			return _hittest;
		}

		void CCaptionButton::SetHitTest(BYTE hittest)
		{
			_hittest = hittest;
		}

		void CCaptionButton::SetBounds(RECT *pRect)
		{
			memcpy(&_bounds, pRect, sizeof(RECT));
		}

		void CCaptionButton::GetBounds(RECT *pRect)
		{
			memcpy(pRect, &_bounds, sizeof(RECT));
		}
	};
};
