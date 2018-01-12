
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
		
		//颜色分布位置，即一个颜色，处理渐变过程中的哪个位置，最大为1
		/*static float positions[] = { 0, 0.45F, 0.5F, 1.0F }; 
		static float positions_down[] = { 0, 0.55F, 0.6F, 1.0F }; 
		static float positions_dis[] = { 0, 1.0F }; */

		CButton::CButton()
			: m_fade(255)
			, m_lbuttondown(false)
			, m_lbuttonup(false)
		{
		}
		CButton::~CButton()
		{
		}

		void CButton::DrawButtonState(Graphics *pGraphics, int width, int height, GraphicsPath *pPath, BYTE alpha, ButtonState bs)
		{
			/* 原来的
			Color bgcolors[4];
			switch (bs)
			{
			case kButtonNormal:
				bgcolors[0] = Color::MakeARGB(alpha,223,225,248);
				bgcolors[1] = Color::MakeARGB(alpha,202,206,241);
				bgcolors[2] = Color::MakeARGB(alpha,208,212,248);
				bgcolors[3] = Color::MakeARGB(alpha,243,250,252);
				break;
			case kButtonHover:
				bgcolors[0] = Color::MakeARGB(alpha,255,251,239);
				bgcolors[1] = Color::MakeARGB(alpha,255,246,220);
				bgcolors[2] = Color::MakeARGB(alpha,255,232,166);
				bgcolors[3] = Color::MakeARGB(alpha,255,252,166);
				break;
			case kButtonDown:
				bgcolors[0] = Color::MakeARGB(alpha,235,231,229);
				bgcolors[1] = Color::MakeARGB(alpha,235,226,210);
				bgcolors[2] = Color::MakeARGB(alpha,235,212,156);
				bgcolors[3] = Color::MakeARGB(alpha,235,232,156);	
				break;
			}*/

			Color *colors = new Color[skin::CSkinData::m_btni.bsi[bs].lc.n];
			for (int i = 0; i < skin::CSkinData::m_btni.bsi[bs].lc.n; i++)
			{
				BYTE alpha_ = Gdip::MixAlpha(skin::CSkinData::m_btni.bsi[bs].lc.colors[i].GetA(), alpha);
				colors[i].SetValue((skin::CSkinData::m_btni.bsi[bs].lc.colors[i].GetValue() & 0x00ffffff) | ((DWORD)alpha_) << Color::AlphaShift);
			}
			LinearGradientBrush lbbg(Gdiplus::Rect(0, 0, width, height),
							Color::WhiteSmoke, Color::Transparent,
							(LinearGradientMode)skin::CSkinData::m_btni.bsi[bs].lc.linear_mode);
			lbbg.SetInterpolationColors(colors, &skin::CSkinData::m_btni.bsi[bs].lc.pos[0], skin::CSkinData::m_btni.bsi[bs].lc.n);
			pGraphics->FillPath(&lbbg, pPath);

			delete[] colors;

			//画Side
			BYTE subalpha = Gdip::MixAlpha(skin::CSkinData::m_btni.bsi[bs].sidecolor.GetA(), alpha);
			if (subalpha == 0)
			{
				return;
			}
			//Pen SidePen(bs == kButtonDown ? Color::MakeARGB(alpha,77,150,200) : Color::MakeARGB(alpha,73,176,222), 2);
			Pen SidePen(((skin::CSkinData::m_btni.bsi[bs].sidecolor.GetValue() & 0x00ffffff) | 
				((DWORD)subalpha) << Color::AlphaShift), 2);
			pGraphics->DrawPath(&SidePen, pPath);
		}

		void CButton::OnDrawBackground(Graphics *pGraphics, int width, int height, GraphicsPath *pPath)
		{
			//pGraphics->Clear(0xffffffff);
			if (m_fade < 255)
			{
				if (m_lbuttonup)
				{
					DrawButtonState(pGraphics, width, height, pPath, 255, kButtonDown);
				} else if (m_bHover && !m_lbuttondown) {
					DrawButtonState(pGraphics, width, height, pPath, 255, kButtonNormal);
				} else {
					DrawButtonState(pGraphics, width, height, pPath, 255, kButtonHover);
				}
			}
			
		}

		void CButton::OnDrawText(Graphics *pGraphics, int width, int height)
		{
			//Text
			std::wstring text;
			int text_len;
			if ((text_len = GetText(text)) > 0)
			{
				pGraphics->DrawString(text.c_str(), text_len, global::Gdip::font,
					RectF(0, 0, (float)width, height), global::Gdip::strfmt, skin::CSkinData::m_btni.text_brush);
			}
		}

		void CButton::OnDrawBody(Graphics *pGraphics, RECT *rect, GraphicsPath *pPath)
		{
			//要渐变的颜色 
			Color fadecolors[4];
			if (m_lbuttondown)
			{
				//按下鼠标
				DrawButtonState(pGraphics, rect->right - rect->left, rect->bottom - rect->top, pPath, m_fade, kButtonDown);
			} else if (m_bHover){
				//鼠标在上
				DrawButtonState(pGraphics, rect->right - rect->left, rect->bottom - rect->top, pPath, m_fade, kButtonHover);
			} else {
				DrawButtonState(pGraphics, rect->right - rect->left, rect->bottom - rect->top, pPath, m_fade, kButtonNormal);
			}
		}

		bool CButton::OnDrawParentMemDC(HDC hMemDC)
		{
			HWND hParentWnd = GetParent();
			HDC hParentMemDC = (HDC)::SendMessage(hParentWnd, WM_TA_GET_MEMORY_HDC, NULL, NULL);
			if (hParentMemDC)
			{
				RECT rc, rcParent;
				if (::GetWindowRect(m_hWnd, &rc) && ::GetWindowRect(hParentWnd, &rcParent))
				{
					/*::ClientToScreen(m_hWnd, (LPPOINT)&rc);
					::ClientToScreen(m_hWnd, &(((LPPOINT)&rc)[1]));
					::ClientToScreen(hParentWnd, (LPPOINT)&rcParent);
					::ClientToScreen(hParentWnd, &(((LPPOINT)&rcParent)[1]));*/

					BitBlt(hMemDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
							hParentMemDC, rc.left - rcParent.left, rc.top - rcParent.top, SRCCOPY);

					return true;
				}
			}
			return false;
		}

		void CButton::OnDrawMemoryDC(HDC hMemDC, int width, int height)
		{
			Graphics g(hMemDC);
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			GraphicsPath round;
			RECT rc = {0, 0, width, height};
			Gdip::MakeRoundRect(&rc, 4, &round);
			
			if (!OnDrawParentMemDC(hMemDC))
			{
				g.Clear(0xffffffff);
			}

			if (IsEnabled())
			{
				//背景
				OnDrawBackground(&g, width, height, &round);
				//主体
				OnDrawBody(&g, &rc, &round);
			}
			else
			{
				//g.Clear(0xffffffff);
				DrawButtonState(&g, width, height, &round, 255, kButtonUnenable);
			}
			
			OnDrawText(&g, width, height);
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

			SkCanvas canvas(bitmap);

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
			case WM_LBUTTONUP:
				if (m_lbuttondown)
				{
					//不知为何，刚按下也会有弹起的消息
					if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
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