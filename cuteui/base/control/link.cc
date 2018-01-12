
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "link.h"

#include "base/view_gdiplus.h"
#include "base/view_skia.h"

#include "base/frame/skin_data.h"

#define ID_TIMER_TEXT_ALPHA 4001

namespace view {
	namespace frame {
		
		CLink::CLink()
			: m_last_state(kLSNormal)
			, m_state(kLSNormal)
			, m_text_alpha(255)
		{
			if (skin::CSkinData::m_li.bload) {
				for (int i = 0; i < kLSCount; i++) {
					m_text_color[i] = skin::CSkinData::m_li.si[i].color.GetValue();
					m_text_underline[i] = skin::CSkinData::m_li.si[i].underline;
				}
			} else {
				m_text_color[kLSNormal] = SK_ColorBLUE;
				m_text_color[kLSHover] = SK_ColorMAGENTA;
				m_text_color[kLSDown] = 0xffdd4b39;

				m_text_underline[kLSNormal] = false;
				m_text_underline[kLSHover] = true;
				m_text_underline[kLSDown] = true;
			}
		}
		CLink::~CLink()
		{
		}

		WindowType CLink::GetType()
		{
			return kWTLink;
		}

		bool CLink::RegisterClass()
		{
			WNDCLASSEX wcex;
			memset(&wcex, 0, sizeof(WNDCLASSEX));

			wcex.cbSize = sizeof(WNDCLASSEX);

			wcex.style			= CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc	= WndProc;
			//wcex.cbClsExtra		= 0;
			//wcex.cbWndExtra		= 0;
			wcex.hInstance		= global::hInstance;
			//wcex.hIcon			= NULL;
			wcex.hCursor		= LoadCursor(NULL, IDC_HAND);
			wcex.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);	//(HBRUSH)(COLOR_WINDOW+1);
			//wcex.lpszMenuName	= NULL;
			wcex.lpszClassName	= kLinkClassName;
			//wcex.hIconSm		= NULL;

			//如果函数失败，返回值为0
			return (RegisterClassEx(&wcex) != 0);
		}

		bool CLink::CreateLink(HWND hParent, LPCWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (Create(hParent, kLinkClassName, lpszText, dwStyle,
				x, y,
				width, height, (HMENU)id))
			{
				SetID(id);
				
				m_width = width;
				m_height = height;
				CreateMemoryDC();

				Show();
				return true;
			} else {
				return false;
			}

			return true;
		}

		void CLink::UIUpdate(bool force_)
		{
			if (m_need_update || force_)
			{
				m_need_update = false;

				SkCanvas canvas(m_skbitmap);
				//Graphics g(m_hMemDC);
				//g.SetSmoothingMode(SmoothingModeAntiAlias);
				//g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

				if (!PaintParent(canvas)) {
					canvas.clear(SK_ColorWHITE);
				}

				std::wstring str;
				if (GetText(str) > 0) {
					int len = str.length();

					SkPaint font_paint;
					font_paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
					font_paint.setTypeface(global::skia::font);
					font_paint.setTextSize(13);
					//font_paint.setFakeBoldText(true);
					font_paint.setTextAlign(SkPaint::kLeft_Align);
					//font_paint.setTextScaleX(1.4f);
					font_paint.setAntiAlias(true);

					SkRect textrect = {0};
					font_paint.measureText(str.c_str(), len << 1, &textrect);

					int left = -textrect.left();
					int top = ((m_height - textrect.height()) / 2) - textrect.top();

					if (m_text_alpha < 255) {

						SkPaint font_last_paint(font_paint);

						font_paint.setColor(m_text_color[m_state]);
						font_last_paint.setColor(m_text_color[m_last_state]);
						if (HaveUnderline(m_state)) {
							font_paint.setUnderlineText(true);
						}
						if (HaveUnderline(m_last_state)) {
							font_last_paint.setUnderlineText(true);
						}

						font_paint.setAlpha(m_text_alpha);
						font_last_paint.setAlpha(255 - m_text_alpha);

						canvas.drawText(str.c_str(), len << 1, 
							left, top, font_last_paint);
						canvas.drawText(str.c_str(), len << 1, 
							left, top, font_paint);
					} else {
						font_paint.setColor(m_text_color[m_state]);
						if (HaveUnderline(m_state)) {
							font_paint.setUnderlineText(true);
						}
						canvas.drawText(str.c_str(), len << 1, 
							left, top, font_paint);
					}
				}
			}
		}

		int CLink::OnMouseMove(bool movein)
		{
			if (movein) {
				SetState();

				m_text_alpha = 1;
				SetTimer(m_hWnd, ID_TIMER_TEXT_ALPHA, kFadeElapse, NULL);
			}
			return 0;
		}

		int CLink::OnMouseLeave()
		{
			SetState();

			m_text_alpha = 1;
			SetTimer(m_hWnd, ID_TIMER_TEXT_ALPHA, kFadeElapse, NULL);
			return 0;
		}

		void CLink::SetState(LinkState state)
		{
			m_last_state = m_state;
			if (state == kLSCount) {
				if (m_bHover) {
					m_state = kLSHover;
				} else {
					m_state = kLSNormal;
				}
			} else {
				m_state = state;
			}
		}

		LRESULT CALLBACK CLink::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			if (handled) return 0;
			switch (message)
			{
			case WM_TIMER:
				{
					switch (wParam)
					{
					case ID_TIMER_TEXT_ALPHA:
						m_text_alpha += kFadeStep;
						if (m_text_alpha > 255) {
							m_text_alpha = 255;
							KillTimer(m_hWnd, ID_TIMER_TEXT_ALPHA);
						}
						m_need_update = true;
						RePaint();
						break;
					}
				}
				break;
			case WM_LBUTTONUP:
				if (IsEnabled()) {

					SetState();

					m_text_alpha = 1;
					SetTimer(m_hWnd, ID_TIMER_TEXT_ALPHA, kFadeElapse, NULL);
					SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(m_id, BN_CLICKED), 0);
				}
				break;
			case WM_LBUTTONDOWN:
				if (IsEnabled()) {

					SetState(kLSDown);

					m_text_alpha = 1;
					SetTimer(m_hWnd, ID_TIMER_TEXT_ALPHA, kFadeElapse, NULL);
				}
				break;
			}
			
			return CCustom::OnWndProc(message, wParam, lParam, handled);
		}

	};
};