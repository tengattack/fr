
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "imagebutton.h"

#include "base/view_gdiplus.h"
#include "base/frame/skin_data.h"

namespace view{
	namespace frame{
		
		CImageButton::CImageButton()
			: m_need_update(true)
			, m_btnimage(NULL)
		{
		}
		CImageButton::~CImageButton()
		{
		}

		void CImageButton::setButtonImage(SkBitmap& image, bool swap) {
			if (!m_btnimage) {
				m_btnimage = new SkBitmap;
			}
			if (swap) {
				m_btnimage->swap(image);
			} else {
				image.deepCopyTo(m_btnimage, SkBitmap::kARGB_8888_Config);
			}

			m_need_update = true;
		}

		void CImageButton::setButtonImage(SkBitmap *image) {
			if (m_btnimage) {
				SkDELETE(m_btnimage);
			}
			m_btnimage = image;
			m_need_update = true;
		}

		void CImageButton::UIUpdate(bool force_)
		{
			if (m_need_update || force_)
			{
				m_need_update = false;

				HDC hdc = ::GetDC(m_hWnd);
				OnPaint(hdc);
				ReleaseDC(m_hWnd, hdc);
			}
		}

		void CImageButton::DrawButtonState(SkCanvas& canvas, SkRect& rect, BYTE alpha, ButtonState bs)
		{
			if (m_btnimage) {
				//SkRect srcrect = {0, 0, m_btnimage.width(), m_btnimage.height()};
				canvas.drawBitmapRectToRect(*m_btnimage, NULL, rect, NULL);
			}

			//画Side
			BYTE subalpha = Gdip::MixAlpha(skin::CSkinData::m_btni.bsi[bs].sidecolor.GetA(), alpha);
			if (subalpha == 0) {
				return;
			}

			/*Pen SidePen(((skin::CSkinData::m_btni.bsi[bs].sidecolor.GetValue() & 0x00ffffff) | 
				((DWORD)subalpha) << Color::AlphaShift), 2);*/

			SkPaint pen;
			pen.setColor(((skin::CSkinData::m_btni.bsi[bs].sidecolor.GetValue() & 0x00ffffff) | 
				((DWORD)subalpha) << Color::AlphaShift));
			
			pen.setStyle(SkPaint::kStroke_Style);
			pen.setStrokeWidth(1.5f);
			pen.setAntiAlias(true);
			//canvas.drawPath(*pPath, pen);

			SkRect rectSideLine = rect;
			rectSideLine.outset(-0.5f, -0.5f);

			canvas.drawRoundRect(rectSideLine, (SkScalar)BUTTON_ROUND_RADIUS, (SkScalar)BUTTON_ROUND_RADIUS, pen);
		}

		void CImageButton::OnDrawBackground(SkCanvas& canvas, SkRect& rect)
		{
			if (m_fade < 255)
			{
				if (m_lbuttonup)
				{
					DrawButtonState(canvas, rect, 255, kButtonDown);
				} else if (m_bHover && !m_lbuttondown) {
					DrawButtonState(canvas, rect, 255, kButtonNormal);
				} else {
					DrawButtonState(canvas, rect, 255, kButtonHover);
				}
			}
		}

		void CImageButton::OnDrawText(SkCanvas& canvas, int width, int height)
		{
		}

		void CImageButton::OnDrawBody(SkCanvas& canvas, SkRect& rect)
		{
			//要渐变的颜色 
			Color fadecolors[4];
			if (m_lbuttondown)
			{
				//按下鼠标
				DrawButtonState(canvas, rect, m_fade, kButtonDown);
			} else if (m_bHover){
				//鼠标在上
				DrawButtonState(canvas, rect, m_fade, kButtonHover);
			} else {
				DrawButtonState(canvas, rect, m_fade, kButtonNormal);
			}
		}

		bool CImageButton::OnDrawParent(SkCanvas& canvas)
		{
			return SkiaPaintParent(m_hWnd, canvas);
		}

		void CImageButton::OnDraw(SkBitmap &bitmap, int width, int height)
		{
			//g.SetSmoothingMode(SmoothingModeAntiAlias);
			SkRect rc = {0, 0, width, height};

			//path: &round for gdi+
			/*SkPath round;
			SkiaMakeRoundRect(&rc, BUTTON_ROUND_RADIUS, &round);*/
			
			SkCanvas canvas(bitmap);

			if (!OnDrawParent(canvas))
			{
				//g.Clear(0xffffffff);
				canvas.clear(SK_ColorWHITE);
			}

			if (IsEnabled())
			{
				//背景
				OnDrawBackground(canvas, rc);
				//主体
				OnDrawBody(canvas, rc);
			}
			else
			{
				//g.Clear(0xffffffff);
				DrawButtonState(canvas, rc, 255, kButtonUnenable);
			}
			
			OnDrawText(canvas, width, height);
		}

		int CImageButton::OnPaint(HDC hdc)
		{
			RECT rc = {0};
			::GetWindowRect(m_hWnd, &rc);
			int width = rc.right - rc.left;
			int height = rc.bottom - rc.top;

			//双缓冲
			//HRGN hRgn = CreateRoundRectRgn(2, 2, width - 2, height - 2, width, height);
#if 0		
			HDC hMemDC = ::CreateCompatibleDC(NULL);
			
			HBITMAP hMemBitmap = ::CreateCompatibleBitmap(hdc, width + 1, height + 1);
			HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hMemBitmap);

			OnDrawMemoryDC(hMemDC, width - 1, height - 1);

			//SetWindowRgn(m_hWnd, hRgn, FALSE);
			::BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);


			//clear
			//::DeleteObject(hRgn);
			::DeleteObject(hOldBitmap);
			::DeleteObject(hMemBitmap);
			::DeleteDC(hMemDC);
#endif
			SkBitmap bitmap;
			bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
			bitmap.allocPixels();
			bitmap.setIsOpaque(true);

			OnDraw(bitmap, width, height);

			SkiaPaint(hdc, 0, 0, width, height, bitmap, 0, 0);

			return 0;
		}


		LRESULT CALLBACK CImageButton::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			return CButton::OnWndProc(message, wParam, lParam, handled);
		}
	};
};