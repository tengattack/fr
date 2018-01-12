
#ifndef _SNOW_CUTE_FRAME_SKIA_H_
#define _SNOW_CUTE_FRAME_SKIA_H_ 1

#include "frame.h"
#include "../view_skia.h"

#include "../common.h"

#define SKIA_LINEAR_MODE_HORIZONTAL			0
#define SKIA_LINEAR_MODE_VERTICAL			1
#define SKIA_LINEAR_MODE_FORWARDDIAGONAL	2
#define SKIA_LINEAR_MODE_BACKWARDDIAGONAL	3

namespace view{
	namespace frame{

		void SkiaMakeRoundRect(RECT* rectangle, int r, SkPath* pPath);
		bool SkiaPaintParent(HWND hWnd, SkCanvas& canvas, bool onlyclient = true);
		bool SkiaPaintParent(HWND hWnd, HDC hdc, bool onlyclient = true);
		void SkiaPaint(void* ctx, int x, int y, int cx, int cy, const SkBitmap& bitmap, int srcx, int srcy);

		void SkiaDrawLinear(SkCanvas& canvas, int linear_mode, const SkColor colors[], const SkScalar pos[], int count, 
			int x, int y, int cx, int cy, SkRect *layout = NULL, int radius = 0);
		void SkiaDrawLinear(SkCanvas& canvas, int linear_mode, const SkColor colors[], const SkScalar pos[], int count, 
			SkRect& layout, int radius);

		class SkiaInterface {

		public:
			SkiaInterface();
			~SkiaInterface();

			void doSkPaint(void* ctx, int x, int y, int cx, int cy, int srcx, int srcy);

			void SkResize(int width, int height, SkBitmap::Config config = SkBitmap::kNo_Config);

			inline const SkBitmap& getBitmap() const
			{
				return m_skbitmap;
			}

		protected:

			SkBitmap m_skbitmap;
			SkBitmap::Config m_skconfig;

		};

	};
};

#endif