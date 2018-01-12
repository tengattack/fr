

#ifndef _SNOW_CUTE_IMAGE_BUTTON_H_
#define _SNOW_CUTE_IMAGE_BUTTON_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "../frame/frame_skia.h"
#include "button.h"

namespace view{
	namespace frame{
		
		class CImageButton : public CButton {

		public:
			CImageButton();
			~CImageButton();

		//protected:
			//virtual void InitWindow();

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			inline SkBitmap& getButtonImage() {
				return *m_btnimage;
			}
			void setButtonImage(SkBitmap& image, bool swap = false);
			void setButtonImage(SkBitmap *image);

			void UIUpdate(bool force_ = false);

		protected:

			bool m_need_update;
			SkBitmap *m_btnimage;

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