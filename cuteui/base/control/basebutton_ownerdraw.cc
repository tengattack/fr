
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "base/frame/skin_data.h"

#include "base/view_gdiplus.h"
#include "base/view_skia.h"

#include "basebutton_ownerdraw.h"

#include <vsstyle.h>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")
 
//radio 和 checkbox 在这部分中前面这些样式是一样的

#define BS_UNCHECKEDNORMAL     0x00000001   
#define BS_UNCHECKEDHOT        0x00000002   
#define BS_UNCHECKEDPRESSED    0x00000003   
#define BS_UNCHECKEDDISABLED   0x00000004   
   
#define BS_CHECKEDNORMAL       0x00000005   
#define BS_CHECKEDHOT          0x00000006   
#define BS_CHECKEDPRESSED      0x00000007   
#define BS_CHECKEDDISABLED     0x00000008

#define BS_UNCHECK_TO_CHECK(state)	((state) + 4)

#define ID_ALPHA_TIMER				4001

namespace view {
	namespace frame {

		#define STYLE_TO_PARTID(style) (style)

		CBaseButtonOwnerDraw::CBaseButtonOwnerDraw(ButtonOwnerDrawStyle style)
			: m_style(style)
			, m_text_color(SK_ColorBLACK)
		{
		}
		CBaseButtonOwnerDraw::~CBaseButtonOwnerDraw()
		{
		}

		WindowType CBaseButtonOwnerDraw::GetType()
		{
			return kWTButton;
		}

		bool CBaseButtonOwnerDraw::CreateButtonOwnerDraw(HWND hParent, LPWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (CreateButton(hParent, lpszText, id, x, y, width, height, dwStyle)) {
				m_width = width;
				m_height = height;

				m_text_color = skin::CSkinData::GetStaticTextColor(hParent);

				CreateMemoryDC();

				return true;
			}
			return false;
		}

		void CBaseButtonOwnerDraw::CreateMemoryDC()
		{
			SkResize(m_width, m_height);
		}

		void CBaseButtonOwnerDraw::SetTextColor(SkColor color)
		{
			m_text_color = color;
			//need repaint
		}

		void CBaseButtonOwnerDraw::OnPaint(HDC hdc)
		{
			OnDrawControl(hdc, m_control_alpha);
			OnDrawText(hdc);
		}

		void CBaseButtonOwnerDraw::OnDrawText(HDC hdc)
		{
			RECT rect = {0};

			::GetClientRect(m_hWnd, &rect);

			rect.left += 15;
			if (rect.left >= rect.right) {
				return;
			}

			SkCanvas canvas(m_skbitmap);

			if (!SkiaPaintParent(m_hWnd, canvas)) {
				canvas.clear(SK_ColorWHITE);
			}

			SkPaint paint;

			paint.setColor(m_text_color);
			paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
			paint.setTypeface(global::skia::font);
			paint.setTextSize(13.0f);
			paint.setTextAlign(SkPaint::kLeft_Align);
			paint.setAntiAlias(true);

			std::wstring str;
			GetText(str);

			SkRect textrect;
			size_t text_len = str.length();
			paint.measureText(str.c_str(), text_len << 1, &textrect);

			int height = rect.bottom - rect.top;
			int width = rect.right - rect.left;

			SkScalar left, top;
			left = 15 - textrect.left();
			top = (height - textrect.height()) / 2 - textrect.top();

			canvas.drawText(str.c_str(), text_len << 1, 
						left, top, paint);

			doSkPaint(hdc, 15, 0, width, height,
				15, 0);
		}

		void CBaseButtonOwnerDraw::OnDrawControl(HDC hdc, BYTE alpha)
		{
			RECT rect = {0}, boxrect = {0};

			::GetClientRect(m_hWnd, &rect);
			memcpy(&boxrect, &rect, sizeof(RECT));
			boxrect.right = boxrect.left + 13;

			HDC hMemDC = ::CreateCompatibleDC(NULL);

			int width = boxrect.right - boxrect.left;
			int height = boxrect.bottom - boxrect.top;
			
			HBITMAP hMemBitmap = ::CreateCompatibleBitmap(hdc, width, height);
			HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hMemBitmap);
			DeleteObject(hOldBitmap);

			HTHEME _hTheme = OpenThemeData(m_hWnd, L"Button");

			int state = 0;
			if (IsEnabled()) {
				if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
					state = BS_UNCHECKEDPRESSED;
				} else if (m_bHover) {
					state = BS_UNCHECKEDHOT;
				} else {
					state = BS_UNCHECKEDNORMAL;
				}
			} else {
				state = BS_UNCHECKEDDISABLED;
			}

			if (GetCheck()) {
				state = BS_UNCHECK_TO_CHECK(state);
			}

			DrawThemeBackground(_hTheme, hMemDC, STYLE_TO_PARTID(m_style), state,
					&boxrect, &boxrect);

			BLENDFUNCTION ftn = {0};
			/*ftn.BlendOp;
			ftn.BlendFlags;*/
			ftn.AlphaFormat = AC_SRC_ALPHA;
			ftn.SourceConstantAlpha = alpha;

			AlphaBlend(hdc, 0, 0, width, height, hMemDC, 0, 0, width, height, ftn);

			DeleteDC(hMemDC);

			//::DrawFrameControl(hdc, &boxrect, DFC_BUTTON, DFCS_BUTTONRADIO | (GetCheck() ? DFCS_CHECKED : 0));  

			CloseThemeData(_hTheme);
		}

		LRESULT CALLBACK CBaseButtonOwnerDraw::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message) {
			case WM_TIMER:
				{
					switch (wParam)
					{
					case ID_ALPHA_TIMER:
						handled = true;

						if (!IsVisible()) {
							m_control_alpha = 255;
							KillTimer(m_hWnd, ID_ALPHA_TIMER);
						}

						m_control_alpha += kFadeStep;
						if (m_control_alpha >= 255) {
							m_control_alpha = 255;
							KillTimer(m_hWnd, ID_ALPHA_TIMER);
						}

						HDC hdc = GetDC(m_hWnd);
						OnDrawControl(hdc, m_control_alpha);
						ReleaseDC(m_hWnd, hdc);
						return 0;
						break;
					}
					break;
				}
			case WM_ERASEBKGND:
				handled = true;
				return 0;
				break;
			case WM_SIZE:
				{
					handled = true;
					m_width = LOWORD(lParam); // width of client area
					m_height = HIWORD(lParam); // height of client area

					CreateMemoryDC();
				}
				break;
				return 0;
			case WM_PAINT:
				{
					handled = true;
					PAINTSTRUCT ps = {0};
					
					//其实不需要这步
					//KillTimer(m_hWnd, ID_ALPHA_TIMER);

					HDC hdc = BeginPaint(m_hWnd, &ps);
					m_control_alpha = 1;
					OnPaint(hdc);
					EndPaint(m_hWnd, &ps);

					SetTimer(m_hWnd, ID_ALPHA_TIMER, kFadeElapse, NULL);
				}
				return 0;
				break;
			}
			return CBaseButton::OnWndProc(message, wParam, lParam, handled);
		}
	};
};