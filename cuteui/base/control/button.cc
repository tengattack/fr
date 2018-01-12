
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "button.h"

#include "base/view_gdiplus.h"
#include "base/frame/skin_data.h"

namespace view{
	namespace frame{

		#define BUTTON_ROUND_RADIUS	4
		
		CButton::CButton()
			: m_fade(255)
			, m_lbuttondown(false)
			, m_lbuttonup(false)
		{
		}
		CButton::~CButton()
		{
		}

		void CButton::DrawButtonState(SkCanvas& canvas, SkRect& rect, BYTE alpha, ButtonState bs)
		{
			Color *colors = new Color[skin::CSkinData::m_btni.bsi[bs].lc.n];
			for (int i = 0; i < skin::CSkinData::m_btni.bsi[bs].lc.n; i++)
			{
				BYTE alpha_ = Gdip::MixAlpha(skin::CSkinData::m_btni.bsi[bs].lc.colors[i].GetA(), alpha);
				colors[i].SetValue((skin::CSkinData::m_btni.bsi[bs].lc.colors[i].GetValue() & 0x00ffffff) | ((DWORD)alpha_) << Color::AlphaShift);
			}
			/*LinearGradientBrush lbbg(Gdiplus::Rect(0, 0, width, height),
							Color::WhiteSmoke, Color::Transparent,
							(LinearGradientMode)skin::CSkinData::m_btni.bsi[bs].lc.linear_mode);
			lbbg.SetInterpolationColors(colors, &skin::CSkinData::m_btni.bsi[bs].lc.pos[0], skin::CSkinData::m_btni.bsi[bs].lc.n);
			pGraphics->FillPath(&lbbg, pPath);
			*/

			SkiaDrawLinear(canvas, skin::CSkinData::m_btni.bsi[bs].lc.linear_mode, 
				(SkColor *)colors, &skin::CSkinData::m_btni.bsi[bs].lc.pos[0], skin::CSkinData::m_btni.bsi[bs].lc.n,
				rect, BUTTON_ROUND_RADIUS);

			delete[] colors;
			//SkScan::FillPath()

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

		void CButton::OnDrawBackground(SkCanvas& canvas, SkRect& rect)
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

		void CButton::OnDrawText(SkCanvas& canvas, int width, int height)
		{
			//Text
			std::wstring text;
			int text_len;
			if ((text_len = GetText(text)) > 0)
			{
				SkPaint paint;
				SkRect textrect;
				paint.setColor(skin::CSkinData::m_btni.text_color.GetValue());
				paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
				paint.setTypeface(global::skia::font);
				paint.setTextSize(13);
				paint.setTextAlign(SkPaint::kLeft_Align);
				paint.setAntiAlias(true);

				paint.measureText(text.c_str(), text_len << 1, &textrect);
				
				canvas.drawText(text.c_str(), text_len << 1, 
					((float)width - textrect.width()) / 2 - textrect.left(), ((float)height + textrect.height()) / 2 - textrect.bottom(), paint);
				/*pCanvas->DrawString(text.c_str(), text_len, global::Gdip::font,
					RectF(0, 0, (float)width, height), global::Gdip::strfmt, skin::CSkinData::m_btni.text_brush);*/
				
			}
		}

		void CButton::OnDrawBody(SkCanvas& canvas, SkRect& rect)
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

		bool CButton::OnDrawParent(SkCanvas& canvas)
		{
			return SkiaPaintParent(m_hWnd, canvas);
		}

		void CButton::OnDraw(SkBitmap &bitmap, int width, int height)
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

		int CButton::OnPaint(HDC hdc)
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

		int CButton::OnMouseMove(bool movein)
		{
			if (movein)
			{
				KillTimer(m_hWnd, global::kTimerFadeOut);
				KillTimer(m_hWnd, global::kTimerFadeDown);

				m_fade = 0;
				SetTimer(m_hWnd, global::kTimerFadeIn, kFadeElapse, NULL);
			}
			return 0;
		}

		int CButton::OnMouseLeave()
		{
			KillTimer(m_hWnd, global::kTimerFadeIn);
			KillTimer(m_hWnd, global::kTimerFadeDown);

			m_fade = 0;
			SetTimer(m_hWnd, global::kTimerFadeOut, kFadeElapse, NULL);
			return 0;
		}

		LRESULT CALLBACK CButton::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{

			switch (message)
			{
			case WM_ERASEBKGND:
			case WM_PAINT:
				handled = true;
				break;
			}

			int result = CChild::OnWndProc(message, wParam, lParam, handled);
			
			/*std::wstring debuginfo;
			base::SStringPrintf(&debuginfo, L"message=0x%02X\r\n", message);
			OutputDebugString(debuginfo.c_str());*/

			switch (message)
			{
			case WM_TIMER:
				{
					switch (wParam)
					{
					case global::kTimerFadeIn:
					case global::kTimerFadeOut:
					case global::kTimerFadeDown:
						m_fade += m_lbuttondown ? (kFadeStep * 2) : kFadeStep;
						if (m_fade >= 255)
						{
							m_fade = 255;
							KillTimer(m_hWnd, wParam);
						}

						HDC hdc = GetDC(m_hWnd);
						OnPaint(hdc);
						ReleaseDC(m_hWnd, hdc);

						if (wParam == global::kTimerFadeIn && m_fade >= 255)
							m_lbuttonup = false;
					break;
					}
				}
				break;
			case WM_LBUTTONUP:
				if (m_lbuttondown)
				{
					//不知为何，刚按下也会有弹起的消息 <-- WM_TIMER 之前没有break！！
					//if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
					{
						m_lbuttondown = false;
						if (IsEnabled())
						{
							m_fade = 0;
							KillTimer(m_hWnd, global::kTimerFadeDown);

							m_lbuttonup = true;
							SetTimer(m_hWnd, global::kTimerFadeIn, kFadeElapse, NULL);
						}
					}
				}
				break;
			case WM_LBUTTONDOWN:
				if (!m_lbuttondown)
				{
					m_lbuttondown = true;
					m_lbuttonup = false;
					if (IsEnabled())
					{
						KillTimer(m_hWnd, global::kTimerFadeIn);
						KillTimer(m_hWnd, global::kTimerFadeOut);
						m_fade = 0;
						SetTimer(m_hWnd, global::kTimerFadeDown, kFadeElapse, NULL);
					}
				}
				break;
			case WM_ERASEBKGND:
				{
					HDC hdc = (HDC)wParam;

					//we handled it.
					OnPaint(hdc);
					return TRUE;
				}
				break;
			case WM_SETTEXT:
				{
					HDC hdc = GetDC(m_hWnd);
					OnPaint(hdc);
					ReleaseDC(m_hWnd, hdc);
					return TRUE;
				}
				break;
			case WM_ENABLE:
				{
					KillTimer(m_hWnd, global::kTimerFadeIn);
					KillTimer(m_hWnd, global::kTimerFadeOut);
					KillTimer(m_hWnd, global::kTimerFadeDown);
					m_fade = 255;

					HDC hdc = GetDC(m_hWnd);
					OnPaint(hdc);
					ReleaseDC(m_hWnd, hdc);
				}
				break;
			case WM_PAINT:
				{
					PAINTSTRUCT ps = {0};
					HDC hdc = BeginPaint(m_hWnd, &ps);

					OnPaint(hdc);

					EndPaint(m_hWnd, &ps);
					//we handled it.
					return TRUE;
				}
				break;
			}
			return result;
		}

		WindowType CButton::GetType()
		{
			return kWTButton;
		}
	};
};