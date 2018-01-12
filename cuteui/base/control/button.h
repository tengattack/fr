

#ifndef _SNOW_CUTE_BUTTON_H_
#define _SNOW_CUTE_BUTTON_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "../frame/frame_skia.h"
#include "basebutton.h"

namespace view{
	namespace frame{
		
		enum ButtonState{
			kButtonNormal = 0,
			kButtonHover,
			kButtonDown,
			kButtonUnenable
		};

		class CButton : public CBaseButton {

		public:
			CButton();
			~CButton();

			static const int kFadeStep = 55;
			static const int kFadeElapse = 60;

		//protected:
			//virtual void InitWindow();

			virtual int OnMouseMove(bool movein);
			virtual int OnMouseLeave();

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
			virtual WindowType GetType();

		protected:
			int m_fade;
			bool m_lbuttondown;
			bool m_lbuttonup;

			virtual void OnDrawBody(SkCanvas& canvas, SkRect& rect);	//, SkPath *pPath
			virtual void OnDrawBackground(SkCanvas& canvas, SkRect& rect);
			virtual void OnDrawText(SkCanvas& canvas, int width, int height);
			//virtual void OnDrawMemoryDC(HDC hMemDC, int width, int height);
			//virtual bool OnDrawParentMemDC(HDC hMemDC);
			virtual void OnDraw(SkBitmap &bitmap, int width, int height);
			virtual bool OnDrawParent(SkCanvas& canvas);

			//DrawState
			virtual void DrawButtonState(SkCanvas& canvas, SkRect& rect, BYTE alpha, ButtonState bs);
			
			virtual int OnPaint(HDC hdc);
		};
	};
};

#endif