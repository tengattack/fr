
#include "frame.h"
#include "frame_skia.h"

#include "../global.h"
#include "../view.h"

namespace view{
	namespace frame{

		bool SkiaPaintParent(HWND hWnd, SkCanvas& canvas, bool onlyclient)
		{
			HWND hParentWnd = ::GetParent(hWnd);
			
			if (onlyclient) {
				//由于主窗口整个bitmap都是，Client只是一部分
				CWindow *pWnd = GetWindow(hParentWnd);
				if (pWnd && pWnd->GetType() == kWTMain) {
					onlyclient = false;
				}
			}

			SkBitmap *parent_bitmap = (SkBitmap *)SendMessage(hParentWnd, WM_TA_GET_SKBITMAP , NULL, NULL);
#if 0
			HDC hParentMemDC = (HDC)SendMessage(hParentWnd, WM_TA_GET_MEMORY_HDC, NULL, NULL);
			if (hParentMemDC)
			{
				RECT rc, rcParent;
				if (::GetWindowRect(m_hWnd, &rc) && ::GetWindowRect(hParentWnd, &rcParent))
				{
					BitBlt(m_hMemDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
						hParentMemDC, rc.left - rcParent.left, rc.top - rcParent.top, SRCCOPY);
				}
#endif
			if (parent_bitmap) {
				RECT rc = {0}, rcParent = {0};
				bool getrect = false;

				if (onlyclient) {
					if (::GetClientRect(hParentWnd, &rcParent)) {
						if (::ClientToScreen(hParentWnd, (LPPOINT)&rcParent)
							&& ::ClientToScreen(hParentWnd, &(((LPPOINT)&rcParent)[1]))) {
								getrect = true;
						}
					}
				} else {
					if (::GetWindowRect(hParentWnd, &rcParent)) {
						getrect = true;
					}
				}

				if (getrect && ::GetWindowRect(hWnd, &rc))
				{
					int32_t left = rc.left - rcParent.left;
					int32_t top = rc.top - rcParent.top;
					int32_t width = rc.right - rc.left;
					int32_t height = rc.bottom - rc.top;
					SkRect srcrect = {left, top, left + width, top + height};
					SkRect dstrect = {0, 0, width, height};
			
					canvas.drawBitmapRectToRect(*parent_bitmap, &srcrect, dstrect, NULL);
					
					return true;
				}
			}
			return false;
		}

		bool SkiaPaintParent(HWND hWnd, HDC hdc, bool onlyclient)
		{
			HWND hParentWnd = ::GetParent(hWnd);

			if (onlyclient) {
				//由于主窗口整个bitmap都是，Client只是一部分
				CWindow *pWnd = GetWindow(hParentWnd);
				if (pWnd && pWnd->GetType() == kWTMain) {
					onlyclient = false;
				}
			}

			SkBitmap *parent_bitmap = (SkBitmap *)SendMessage(hParentWnd, WM_TA_GET_SKBITMAP , NULL, NULL);
#if 0
			HDC hParentMemDC = (HDC)SendMessage(hParentWnd, WM_TA_GET_MEMORY_HDC, NULL, NULL);
			if (hParentMemDC)
			{
				RECT rc, rcParent;
				if (::GetWindowRect(m_hWnd, &rc) && ::GetWindowRect(hParentWnd, &rcParent))
				{
					BitBlt(m_hMemDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
						hParentMemDC, rc.left - rcParent.left, rc.top - rcParent.top, SRCCOPY);
				}
#endif
			if (parent_bitmap) {
				RECT rc = {0}, rcParent = {0};
				bool getrect = false;

				if (onlyclient) {
					if (::GetClientRect(hParentWnd, &rcParent)) {
						if (::ClientToScreen(hParentWnd, (LPPOINT)&rcParent)
							&& ::ClientToScreen(hParentWnd, &(((LPPOINT)&rcParent)[1]))) {
								getrect = true;
						}
					}
				} else {
					if (::GetWindowRect(hParentWnd, &rcParent)) {
						getrect = true;
					}
				}

				if (getrect && ::GetWindowRect(hWnd, &rc))
				{
					/*BitBlt(m_hMemDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
						hParentMemDC, rc.left - rcParent.left, rc.top - rcParent.top, SRCCOPY);*/
					int32_t left = rc.left - rcParent.left;
					int32_t top = rc.top - rcParent.top;
					SkiaPaint(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
							*parent_bitmap, left, top);
					//canvas.drawBitmapRectToRect(*parent_bitmap, &srcrect, dstrect, NULL);
					
					return true;
				}
			}
			return false;
		}

		void SkiaMakeRoundRect(RECT* rectangle, int r, SkPath* pPath)
		{	
			if (!pPath)
				return;
			int l = 2 * r;

			SkPoint pointlist[] = {
				{rectangle->left + r, rectangle->top},
				{rectangle->right - r, rectangle->top},

				{rectangle->right, rectangle->top + r}, 
				{rectangle->right, rectangle->bottom - r},

				{rectangle->right - r, rectangle->bottom},
				{rectangle->left + r, rectangle->bottom},

				{rectangle->left, rectangle->bottom - r},
				{rectangle->left, rectangle->top + r}
			};

			SkRect rectlist[] = {
				{rectangle->right - l, rectangle->top, rectangle->right, rectangle->top + l},
				{rectangle->right - l, rectangle->bottom - l, rectangle->right, rectangle->bottom},
				{rectangle->left, rectangle->bottom - l, rectangle->left + l, rectangle->bottom},
				{rectangle->left, rectangle->top, rectangle->left + l, rectangle->top + l}
			};

			pPath->addPoly(&pointlist[0], 2, true);
			pPath->addArc(rectlist[0], 270.0f, 90.0f);
			pPath->addPoly(&pointlist[2], 2, true);
            pPath->addArc(rectlist[1], 0.0f, 90.0f);
			pPath->addPoly(&pointlist[4], 2, true);
            pPath->addArc(rectlist[2], 90.0f, 90.0f);
			pPath->addPoly(&pointlist[6], 2, true);
            pPath->addArc(rectlist[3], 180.0f, 90.0f);

			return;
		}

		void SkiaPaint(void* ctx, int x, int y, int cx, int cy, const SkBitmap& bitmap, int srcx, int srcy) {
			//this->update(NULL);

			//if (kNone_BackEndType == fAttached)
			{
				HDC hdc = (HDC)ctx;

				BITMAPINFO bmi;
				memset(&bmi, 0, sizeof(bmi));
				bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
				bmi.bmiHeader.biWidth       = bitmap.width();
				bmi.bmiHeader.biHeight      = -bitmap.height(); // top-down image
				bmi.bmiHeader.biPlanes      = 1;
				bmi.bmiHeader.biBitCount    = 32;
				bmi.bmiHeader.biCompression = BI_RGB;
				bmi.bmiHeader.biSizeImage   = 0;

				//
				// Do the SetDIBitsToDevice.
				//
				// TODO(wjmaclean):
				//       Fix this call to handle SkBitmaps that have rowBytes != width,
				//       i.e. may have padding at the end of lines. The SkASSERT below
				//       may be ignored by builds, and the only obviously safe option
				//       seems to be to copy the bitmap to a temporary (contiguous)
				//       buffer before passing to SetDIBitsToDevice().
				SkASSERT(bitmap.width() * bitmap.bytesPerPixel() == bitmap.rowBytes());
				bitmap.lockPixels();
				int iRet = SetDIBitsToDevice(hdc,
					x, y,
					cx, cy,
					srcx, 0,
					0, cy,
					(byte *)bitmap.getPixels() + bitmap.rowBytes() * srcy,
					&bmi,
					DIB_RGB_COLORS);
				bitmap.unlockPixels();
			}
		}

		void SkiaDrawLinear(SkCanvas& canvas, int linear_mode, const SkColor colors[], const SkScalar pos[], int count, 
			SkRect& layout, int radius)
		{
			SkPaint paint;
			SkShader *shader;

			SkPoint pts[2];

			switch (linear_mode) {
			case SKIA_LINEAR_MODE_HORIZONTAL:
				pts[0].set(layout.left(), layout.top());
				pts[1].set(layout.right(), layout.top());
				break;
			case SKIA_LINEAR_MODE_VERTICAL:
				pts[0].set(layout.left(), layout.top());
				pts[1].set(layout.left(), layout.bottom());
				break;
			case SKIA_LINEAR_MODE_FORWARDDIAGONAL:
				pts[0].set(layout.left(), layout.top());
				pts[1].set(layout.right(), layout.bottom());
				break;
			case SKIA_LINEAR_MODE_BACKWARDDIAGONAL:
				pts[0].set(layout.left(), layout.bottom());
				pts[1].set(layout.right(), layout.top());
				break;
			default:
				return;
			}

			shader = SkGradientShader::CreateLinear(
				pts, 
				colors, pos, count,
				SkShader::kMirror_TileMode);
			//pts决定Gradient的起始点和走向，以及过渡完gClors的颜色需要的像素距离
			paint.setShader(shader);
			shader->unref();

			paint.setAntiAlias(true);

			if (radius > 0) {
				canvas.drawRoundRect(layout, (SkScalar)radius, (SkScalar)radius, paint);
			} else {
				canvas.drawRect(layout, paint);
			}
		}

		void SkiaDrawLinear(SkCanvas& canvas, int linear_mode, const SkColor colors[], const SkScalar pos[], int count, 
			int left, int top, int right, int bottom, SkRect *layout, int radius)
		{
			SkPaint paint;
			SkShader *shader;

			SkPoint pts[2];

			switch (linear_mode) {
			case SKIA_LINEAR_MODE_HORIZONTAL:
				pts[0].set(left, top);
				pts[1].set(right, top);
				break;
			case SKIA_LINEAR_MODE_VERTICAL:
				pts[0].set(left, top);
				pts[1].set(left, bottom);
				break;
			case SKIA_LINEAR_MODE_FORWARDDIAGONAL:
				pts[0].set(left, top);
				pts[1].set(right, bottom);
				break;
			case SKIA_LINEAR_MODE_BACKWARDDIAGONAL:
				pts[0].set(left, bottom);
				pts[1].set(right, top);
				break;
			default:
				return;
			}

			shader = SkGradientShader::CreateLinear(
				pts, 
				colors, pos, count,
				SkShader::kMirror_TileMode);
			//pts决定Gradient的起始点和走向，以及过渡完gClors的颜色需要的像素距离
			paint.setShader(shader);
			shader->unref();

			paint.setAntiAlias(true);

			if (layout) {
				if (radius > 0) {
					canvas.drawRoundRect(*layout, (SkScalar)radius, (SkScalar)radius, paint);
				} else {
					canvas.drawRect(*layout, paint);
				}
			} else {
				SkRect r = {left, top, right, bottom};
				if (radius > 0) {
					canvas.drawRoundRect(r, (SkScalar)radius, (SkScalar)radius, paint);
				} else {
					canvas.drawRect(r, paint);
				}
			}
		}

		SkiaInterface::SkiaInterface()
		{
			m_skconfig = SkBitmap::kARGB_8888_Config;
		}

		SkiaInterface::~SkiaInterface()
		{
		}

		void SkiaInterface::SkResize(int width, int height, SkBitmap::Config config)
		{

			if (config == SkBitmap::kNo_Config)
				config = m_skconfig;

			if (width != m_skbitmap.width() || height != m_skbitmap.height() || config != m_skconfig)
			{
				m_skconfig = config;
				m_skbitmap.setConfig(config, width, height);
				m_skbitmap.allocPixels();
				//m_skbitmap.setIsOpaque(true);

				//this->setSize(SkIntToScalar(width), SkIntToScalar(height));
				//this->inval(NULL);
				//m_height = height;
				//m_width = width;
			}
		}

		void SkiaInterface::doSkPaint(void* ctx, int x, int y, int cx, int cy, int srcx, int srcy)
		{
			SkiaPaint(ctx, x, y, cx, cy, this->getBitmap(), srcx, srcy);
		}

	};
};
