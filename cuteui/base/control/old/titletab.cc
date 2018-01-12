
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "titletab.h"

#include "base/view_gdiplus.h"
#include "base/frame/skin_data.h"

#include "base/string/stringprintf.h"

#define TT_TIMER_ELAPSE		60
#define TT_TIMER_STEP		55
#define TT_TIMER_BASE		1000

namespace view{
	namespace frame{

		static const float positions[] = { 0, 1.0f };
		//static const float _positions[] = { 0, 0.45F, 0.5F, 1.0F }; 

		CTitleTab::CTitleTab()
			: m_hovered_(-1)
			, m_selected_(-1)
			, m_downed_(-1)
			, m_lbuttondown(false)
			, m_lbuttonup(true)
			, m_allow_nselect(true)
		{
		}
		CTitleTab::~CTitleTab()
		{
		}

		WindowType CTitleTab::GetType()
		{
			return kWTTitleTab;
		}

		bool CTitleTab::RegisterClass()
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
			wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);	//(HBRUSH)(COLOR_WINDOW+1);
			//wcex.lpszMenuName	= NULL;
			wcex.lpszClassName	= kTitleTabClassName;
			//wcex.hIconSm		= NULL;

			//如果函数失败，返回值为0
			return (RegisterClassEx(&wcex) != 0);
		}

		LRESULT CALLBACK CTitleTab::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			if (handled) return 0;
			switch (message)
			{
			case WM_TIMER:
				{
					//ID wParam
					int iIndex = -1;
					if ((iIndex = wParam - TT_TIMER_BASE) >= 0)
					{
						//---------------
						TA_TITLE_TAB_ITEM *pItem = &m_tab[iIndex];
						pItem->fade += TT_TIMER_STEP;
						if (pItem->fade >= 255)
						{
							pItem->fade = 255;
							KillTimer(m_hWnd, wParam);
						}

						DrawTabs(&iIndex, 1);
					}
				}
				break;
			case WM_LBUTTONUP:
				if (m_lbuttondown)
				{
					//不知为何，刚按下也会有弹起的消息
					if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
					{
						m_lbuttondown = false;
						if (IsEnabled())
						{
							/*m_fade = 0;
							KillTimer(m_hWnd, global::kTimerFadeDown);*/

							m_lbuttonup = true;
							//SetTimer(m_hWnd, global::kTimerFadeIn, kFadeElapse, NULL);
							SelChange(m_downed_);	//TestSelChange
							int _drawindex = m_downed_;
							SetState(-1, kTTSDown);
							TimerMessages(&_drawindex, 1);
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
						//x point
						int i = XHitTest(LOWORD(lParam));
						if (i != m_downed_)
						{
							int _drawindex[2] = {m_downed_, i};
							SetState(i, kTTSDown);
							TimerMessages(_drawindex, 2);
						}
						/*KillTimer(m_hWnd, global::kTimerFadeIn);
						KillTimer(m_hWnd, global::kTimerFadeOut);
						m_fade = 0;
						SetTimer(m_hWnd, global::kTimerFadeDown, kFadeElapse, NULL);*/
					}
				}
				break;
			case WM_PAINT:
				{
					UIUpdate();

					handled = true;
					PAINTSTRUCT ps = {0};
					HDC hdc = BeginPaint(m_hWnd, &ps);
					int *_drawindex = (int *)malloc(sizeof(int) * m_tab.size());
					if (_drawindex)
					{
						for (int i = 0; i < (int)m_tab.size(); i++)
						{
							_drawindex[i] = i;
						}
						DrawTabs(_drawindex, m_tab.size(), hdc);
						free(_drawindex);
					}
					EndPaint(m_hWnd, &ps);
				}
				break;
			}
			return CCustom::OnWndProc(message, wParam, lParam, handled);
		}

		bool CTitleTab::CreateTitleTab(HWND hParent, LPWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (Create(hParent, kTitleTabClassName, lpszText, dwStyle,
				x, y,
				width, height, (HMENU)id))
			{
				SetID(id);
				//自定义控件无需SubClassWindow();
				//SubClassWindow();
				
				m_width = width;
				m_height = height;
				CreateMemoryDC();
				SkResize(m_width, m_height);

				Show();
				return true;
			} else {
				return false;
			}

			return true;
		}

		//function
		int CTitleTab::AddTab(LPCWSTR lpszText, int width)
		{
			TA_TITLE_TAB_ITEM ttti;
			ttti.text = lpszText;
			ttti.width = width;
			ttti.left = 0;
			ttti.fade = 255;
			ttti.state = kTTSNon;
			ttti.prevstate = kTTSNon;
			m_tab.push_back(ttti);

			m_need_update = true;
			return m_tab.size() - 1;
		}

		void CTitleTab::DeleteTab(int iIndex)
		{
			m_tab.erase(m_tab.begin() + iIndex);
			m_need_update = true;
		}

		void CTitleTab::SetCurSel(int iIndex)
		{
			if (m_selected_ != iIndex)
			{
				int _drawindex[2] = {m_selected_, iIndex};
				SetState(iIndex, kTTSSelect);
				DrawTabs(_drawindex, 2);
			}
		}

		int CTitleTab::GetCurSel()
		{
			return m_selected_;
		}

		void CTitleTab::UIUpdate(bool force_)
		{
			if (m_need_update || force_)
			{
				m_need_update = false;

				//Graphics g(m_hMemDC);
				//g.SetSmoothingMode(SmoothingModeAntiAlias);
				//g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

				SkCanvas canvas(m_skbitmap);

				if (!PaintParent(canvas)) {
					canvas.clear(SK_ColorWHITE);
				}

				SkPaint font_paint;
				font_paint.setColor(skin::CSkinData::m_tti.background_text_color.GetValue());
				font_paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
				font_paint.setTypeface(global::skia::font);
				font_paint.setTextSize(18);
				font_paint.setFakeBoldText(true);
				font_paint.setTextAlign(SkPaint::Align::kLeft_Align);
				//font_paint.setTextScaleX(1.4f);
				font_paint.setAntiAlias(true);

				int text_left = 0;
				for (unsigned long i = 0; i < m_tab.size(); i++)
				{
					int text_length = m_tab[i].text.length();
					//calc width
					SkRect textrect;
					font_paint.measureText(m_tab[i].text.c_str(), text_length << 1, &textrect);

					if (m_tab[i].width == -1) {
						//set width
						m_tab[i].width = (textrect.width() + 24.0f);
						/*
						RectF output;
						if (g.MeasureString(m_tab[i].text.c_str(), text_length, 
							global::Gdip::font_titletab,
							RectF(0, 0, m_width, m_height),
							&output) == Gdiplus::Ok)
						{
							m_tab[i].width = (output.Width + 24.0f);
						} else {
							//m_tab[i].width = 60;
							continue;
						}
						*/
					}

					m_tab[i].left = text_left;

					Color backcolors[] = {  
						skin::CSkinData::m_tti.colors[i % skin::CSkinData::m_tti.colors.size()],
						0x00ffffff,
					};

					/*RectF layout(text_left, 1, m_tab[i].width, m_height - 1);
					
					LinearGradientBrush lbback(layout,
									Color::WhiteSmoke, Color::Transparent,
									LinearGradientModeVertical);
					lbback.SetInterpolationColors(backcolors, positions, 2);
					g.FillRectangle(&lbback, layout);

					Bitmap *bmptext = NULL;
					Gdip::CreateGlowTextBitmap(&bmptext, global::Gdip::font, global::Gdip::fontfamily, (FontStyle)0, 13 * 1.4f, global::Gdip::strfmt, 
							m_tab[i].text.c_str(), text_length,
							m_tab[i].width - 1, m_height - 1,
							skin::CSkinData::m_tti.background_text_color,
							1.0f, 0.5f, false,
							NULL,
							0.0, 1.0);

					g.DrawImage(bmptext, layout);
					delete bmptext;
					
					*/

					SkRect layout = {text_left, 1, text_left + m_tab[i].width, m_height};

					SkPaint paint;
					SkShader *shader;
					SkPoint p = SkPoint::Make(0, 1);
					SkPoint q = SkPoint::Make(0, m_height);

					SkPoint pts[] ={p, q};
					shader = SkGradientShader::CreateLinear(
						pts, 
						(SkColor *)backcolors, positions, 2,
						SkShader::kMirror_TileMode);
					//pts决定Gradient的起始点和走向，以及过渡完gClors的颜色需要的像素距离
					paint.setShader(shader);
					shader->unref();
					paint.setAntiAlias(true);
					canvas.drawRect(layout, paint);

					canvas.drawText(m_tab[i].text.c_str(), text_length << 1, 
						m_tab[i].left + ((float)m_tab[i].width - textrect.width()) / 2 - textrect.left(), ((float)m_height + textrect.height()) / 2 - textrect.bottom(), font_paint);

					text_left += m_tab[i].width + 1;	//间隔1
				}
			}
		}

		void CTitleTab::DrawSelectTab(SkCanvas& canvas, TA_TITLE_TAB_ITEM *pItem, BYTE alpha, int iIndex)
		{
#if 0
			//path
			GraphicsPath round;
			Rect rc(1, 1, pItem->width + 2, m_height + 2);
#if 0
			//只画左上的边框
			//RectF
			//Gdip::MakeRoundRect(&rc, 4, &round);
			Color *colors = new Color[skin::CSkinData::m_tti.select_lcolor.n];
			for (int i = 0; i < skin::CSkinData::m_tti.select_lcolor.n; i++)
			{
				BYTE alpha_ = Gdip::MixAlpha(skin::CSkinData::m_tti.select_lcolor.colors[i].GetA(), (BYTE)alpha);
				/*BYTE subalpha = skin::CSkinData::m_tti.select_lcolor.colors[i].GetA();
				if (subalpha != 255) {
							//非饱和
					subalpha = (BYTE)((float)alpha * ((float)subalpha / 255));
				} else {
					subalpha = alpha;
				}*/
				colors[i].SetValue((skin::CSkinData::m_tti.select_lcolor.colors[i].GetValue() & 0x00ffffff) | ((DWORD)alpha_) << Color::AlphaShift);
			}

			LinearGradientBrush lbbg(Gdiplus::Rect(2, 2, pItem->width-4, m_height-4),
					Color::WhiteSmoke, Color::Transparent,
					(LinearGradientMode)skin::CSkinData::m_tti.select_lcolor.linear_mode);
			lbbg.SetInterpolationColors(colors,
				&skin::CSkinData::m_tti.select_lcolor.pos[0],
				skin::CSkinData::m_tti.select_lcolor.n);
			g->FillPath(&lbbg, &round);

			delete[] colors;
#endif
			//text
			g->DrawString(pItem->text.c_str(), -1, global::Gdip::font_titletab,
				RectF(0, 1, pItem->width, m_height), global::Gdip::strfmt, skin::CSkinData::m_tti.select_text_brush);

			//side
			Color sidecolor = (((DWORD)(alpha * skin::CSkinData::m_tti.select_sidecolor.GetA() / 255)) << Color::AlphaShift) | 
				(skin::CSkinData::m_tti.select_sidecolor.GetValue() & 0x00ffffff);
			/*if (m_lbuttondown)
			{
				sidecolor = Color::MakeARGB(alpha,77,150,200);
			} else {
				sidecolor = Color::MakeARGB(alpha,73,176,222);
			}*/

			Pen SidePen(sidecolor, 2);
			//g->DrawPath(&SidePen, &round);
			g->DrawRectangle(&SidePen, rc);
#endif

			SkRect rc = {1, 1, pItem->width + 2, m_height + 2};

			Color text_color;
			skin::CSkinData::m_tti.select_text_brush->GetColor(&text_color);

			SkPaint paint;
			paint.setColor(text_color.GetValue());
			paint.setAntiAlias(true);
			paint.setFakeBoldText(true);
			paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
			paint.setTypeface(global::skia::font);
			paint.setTextAlign(SkPaint::Align::kLeft_Align);
			paint.setTextSize(18);

			int text_len = pItem->text.length();

			SkRect textrect;

			paint.measureText(pItem->text.c_str(), text_len << 1, &textrect);

			canvas.drawText(pItem->text.c_str(), text_len << 1, 
				((float)pItem->width - textrect.width()) / 2 - textrect.left(), ((float)m_height + textrect.height()) / 2 - textrect.bottom(), paint);

			SkColor sidecolor = (((DWORD)(alpha * skin::CSkinData::m_tti.select_sidecolor.GetA() / 255)) << Color::AlphaShift) | 
				(skin::CSkinData::m_tti.select_sidecolor.GetValue() & 0x00ffffff);

			SkPaint pen;
			pen.setColor(sidecolor);
			pen.setStyle(SkPaint::kStroke_Style);
			pen.setStrokeWidth(2);
			pen.setAntiAlias(true);
			canvas.drawRect(rc, pen);
		}

		void CTitleTab::DrawHoverTab(SkCanvas& canvas, TA_TITLE_TAB_ITEM *pItem, BYTE alpha, int iIndex)
		{
			Color backcolors[] = {  
					(((DWORD)(alpha * 0xa0 / 0xff)) << Gdiplus::Color::AlphaShift) | (skin::CSkinData::m_tti.colors[iIndex % skin::CSkinData::m_tti.colors.size()].GetValue() & 0x00ffffff),
						Color::MakeARGB(0,255,255,255),
			};
					
			/*LinearGradientBrush lbback(Rect(0, 0, pItem->width, m_height),
							Color::WhiteSmoke, Color::Transparent,
							LinearGradientModeVertical);
			lbback.SetInterpolationColors(backcolors, positions, 2);
			g->FillRectangle(&lbback, 0, 0, pItem->width, m_height);*/
			SkPaint paint;
			SkShader *shader;

			SkPoint p = SkPoint::Make(0, 0);
			SkPoint q = SkPoint::Make(0, m_height);
			SkPoint pts[] ={p, q};
			shader = SkGradientShader::CreateLinear(
				pts, 
				(SkColor *)backcolors, positions, 2,
				SkShader::kMirror_TileMode);
			//pts决定Gradient的起始点和走向，以及过渡完gClors的颜色需要的像素距离
			paint.setShader(shader);
			shader->unref();

			paint.setAntiAlias(true);
			SkRect r = {0, 0, SkIntToScalar(pItem->width), SkIntToScalar(m_height)};
			canvas.drawRect(r, paint);
		}

		void CTitleTab::DrawDownTab(SkCanvas& canvas, TA_TITLE_TAB_ITEM *pItem, BYTE alpha, int iIndex)
		{
			if (alpha < 255 && m_hovered_ == iIndex)
				DrawHoverTab(canvas, pItem, 255-alpha, iIndex);

			Color backcolors[] = {  
				((DWORD)alpha << Gdiplus::Color::AlphaShift) | (skin::CSkinData::m_tti.colors[iIndex % skin::CSkinData::m_tti.colors.size()].GetValue() & 0x00ffffff),
				((DWORD)(alpha * 0x30 / 0xff) << Gdiplus::Color::AlphaShift) | (skin::CSkinData::m_tti.colors[iIndex % skin::CSkinData::m_tti.colors.size()].GetValue() & 0x00ffffff),
			};
					
			/*LinearGradientBrush lbback(Rect(0, 0, pItem->width, m_height),
							Color::WhiteSmoke, Color::Transparent,
							LinearGradientModeVertical);
			lbback.SetInterpolationColors(backcolors, positions, 2);
			g->FillRectangle(&lbback, 0, 0, pItem->width, m_height);*/

			SkPaint paint;
			SkShader *shader;

			SkPoint p = SkPoint::Make(0, 0);
			SkPoint q = SkPoint::Make(0, m_height);
			SkPoint pts[] ={p, q};
			shader = SkGradientShader::CreateLinear(
				pts, 
				(SkColor *)backcolors, positions, 2,
				SkShader::kMirror_TileMode);
			//pts决定Gradient的起始点和走向，以及过渡完gClors的颜色需要的像素距离
			paint.setShader(shader);
			shader->unref();

			paint.setAntiAlias(true);
			SkRect r = {0, 0, SkIntToScalar(pItem->width), SkIntToScalar(m_height)};
			canvas.drawRect(r, paint);
		}

		void CTitleTab::DrawTabs(int* pIndexs, int count, HDC hdc)
		{
			HDC _hdc;
			if (hdc)
				_hdc = hdc;
			else
				_hdc = ::GetDC(m_hWnd);

			TA_TITLE_TAB_ITEM *pItem = NULL;

			for (int i = 0; i < count; i++)
			{
				if (pIndexs[i] == -1) continue;
				pItem = &m_tab[pIndexs[i]];	

				if (pIndexs[i] != m_downed_ && pIndexs[i] != m_hovered_ && pIndexs[i] != m_selected_)
				{
					if (pItem->fade >= 255)
					{
						//::BitBlt(_hdc, pItem->left, 0, pItem->width + 1, m_height, m_hMemDC, pItem->left, 0, SRCCOPY);
						doSkPaint(_hdc, pItem->left, 0, pItem->width + 1, m_height, pItem->left, 0);
						continue;
					}
				}

				HDC m_hTabDC = ::CreateCompatibleDC(NULL);
				HBITMAP m_hTabBitmap = ::CreateCompatibleBitmap(_hdc, pItem->width + 1, m_height);
				HBITMAP hOldBitmap = (HBITMAP)::SelectObject(m_hTabDC, m_hTabBitmap);
				DeleteObject(hOldBitmap);

				::BitBlt(m_hTabDC, 0, 0, pItem->width + 1, m_height, m_hMemDC, pItem->left, 0, SRCCOPY);

				SkBitmap tabbitmap;
				tabbitmap.setConfig(SkBitmap::kARGB_8888_Config, pItem->width + 1, m_height);
				tabbitmap.allocPixels();

				SkCanvas canvas(tabbitmap);
				SkRect srcrect = {pItem->left, 0, pItem->left + pItem->width + 1, m_height};
				SkRect dstrect = {0, 0, pItem->width + 1, m_height};
				canvas.drawBitmapRectToRect(m_skbitmap, &srcrect, dstrect, NULL);

				Graphics g(m_hTabDC);
				g.SetSmoothingMode(SmoothingModeAntiAlias);
				// && pIndexs[i] != m_selected_
				if (pIndexs[i] != m_downed_ && pIndexs[i] != m_hovered_)
				{
					//都不是
					if (pItem->prevstate == kTTSHover){
						DrawHoverTab(canvas, pItem, (BYTE)(255 - pItem->fade), pIndexs[i]);
					} else if (pItem->prevstate == kTTSNon){
						if (m_selected_ != pIndexs[i] && m_downed_ == -1 && !m_lbuttondown)
						{
							BYTE alpha  = (BYTE)(255 - (BYTE)(pItem->fade * skin::CSkinData::m_tti.select_alpha / 0xff));
							DrawSelectTab(canvas, pItem, alpha, pIndexs[i]);
						} else {
							DrawDownTab(canvas, pItem, 255 - pItem->fade, pIndexs[i]);
						}
					}

				} else {

					if (m_downed_ == pIndexs[i])
					{
						//鼠标按下
						DrawDownTab(canvas, pItem, pItem->fade, pIndexs[i]);
					} else if (m_hovered_ == pIndexs[i]){
						//鼠标盘旋
						DrawHoverTab(canvas, pItem, pItem->fade, pIndexs[i]);
					}
				
				}

				if (m_selected_ == pIndexs[i])
				{
					//g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

					//Select
					BYTE alpha;
					if (m_downed_ == pIndexs[i])
					{
						alpha = (BYTE)(255 - (BYTE)(pItem->fade * skin::CSkinData::m_tti.select_alpha / 0xff));
					} else if ((m_hovered_ != pIndexs[i] || pItem->prevstate == kTTSNon) && m_lbuttonup){
						alpha = skin::CSkinData::m_tti.select_alpha;
					} else {
						alpha = (BYTE)(pItem->fade * skin::CSkinData::m_tti.select_alpha / 0xff);
					}

					DrawSelectTab(canvas, pItem, alpha, pIndexs[i]);
				}


				//::BitBlt(_hdc, pItem->left, 0, pItem->width + 1, m_height, m_hTabDC, 0, 0, SRCCOPY);
				SkiaPaint(_hdc, pItem->left, 0, pItem->width + 1, m_height, tabbitmap, 0, 0);

				::DeleteObject(m_hTabBitmap);
				::DeleteDC(m_hTabDC);
			}
			
			if (!hdc)
				::ReleaseDC(m_hWnd, _hdc);
		}

		int CTitleTab::OnMouseMove(bool movein, int x, int y)
		{
			bool _find = false;
			int i = XHitTest(x);
			if (movein)
			{
				m_lbuttonup = true;
				m_lbuttondown = false;
			}
			if (i == -1)
			{
				OnMouseLeave();
			}
			else if (m_hovered_ != i)
			{
				int _drawindex[3] = {m_hovered_, i, -1};
				if (m_lbuttondown && m_downed_ != i)
				{
					if (m_downed_ != m_hovered_)
					{
						_drawindex[2] = m_downed_;
					}
					SetState(i, kTTSDown);
				}
				if (!m_lbuttondown && m_downed_ != -1)
				{
					m_downed_ = -1;
				}
				SetState(i, kTTSHover);
				TimerMessages(_drawindex, 3);
			}
			return 0;
		}

		int CTitleTab::OnMouseLeave()
		{
			int _drawindex[2] = {-1, -1};
			bool _need_draw = false;
			if (m_hovered_ != -1)
			{
				_drawindex[0] = m_hovered_;
				SetState(-1, kTTSHover);
				_need_draw = true;
			}
			if (m_downed_ != -1)
			{
				if (m_downed_ != _drawindex[0])
				{
					_drawindex[1] = m_downed_;
				}
				
				SetState(-1, kTTSDown);
				_need_draw = true;
			}
			if (_need_draw)
			{
				//重绘
				TimerMessages(_drawindex, 2);
			}

			
			return 0;
		}

		int CTitleTab::XHitTest(int x)
		{
			for (unsigned long i = 0; i < m_tab.size(); i++)
			{
				if (x >= m_tab[i].left && x <= m_tab[i].left + m_tab[i].width)
				{
					return i;
					break;
				}
			}
			return -1;
		}

		void CTitleTab::SelChange(int iSelIndex)
		{
			if (m_selected_ != iSelIndex)
			{
				if (!(!m_allow_nselect && iSelIndex == -1))
				{
					int _drawindex[2] = {m_selected_, iSelIndex};
					SetState(iSelIndex, kTTSSelect);
					//Message
					TimerMessages(_drawindex, 2);

					//向父窗口发出消息
					SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(m_id, TT_SELCHANGE), m_selected_);
				}
			}
		}

		void CTitleTab::TimerMessages(int* pIndexs, int count)
		{
			TA_TITLE_TAB_ITEM *pItem = NULL;

			for (int i = 0; i < count; i++)
			{
				if (pIndexs[i] == -1) continue;
				pItem = &m_tab[pIndexs[i]];

				KillTimer(m_hWnd, TT_TIMER_BASE + pIndexs[i]);

				pItem->fade = 0;
				SetTimer(m_hWnd, TT_TIMER_BASE + pIndexs[i], TT_TIMER_ELAPSE, NULL);
			}
		}

		void CTitleTab::SetState(int iNewIndex, TitleTabState state)
		{
			switch (state)
			{
			case kTTSNon:
				break;
			case kTTSHover:
				if (m_hovered_ != -1)
				{
					m_tab[m_hovered_].prevstate = m_tab[m_hovered_].state;
					m_tab[m_hovered_].state = kTTSNon;
				}
				m_hovered_ = iNewIndex;
				break;
			case kTTSDown:
				if (m_downed_ != -1)
				{
					m_tab[m_downed_].prevstate = m_tab[m_downed_].state;
					m_tab[m_downed_].state = kTTSNon;
				}
				m_downed_ = iNewIndex;
				break;
			case kTTSSelect:
				if (!m_allow_nselect && iNewIndex == -1)
					break;

				if (m_selected_ != -1)
				{
					m_tab[m_selected_].prevstate = m_tab[m_selected_].state;
					m_tab[m_selected_].state = kTTSNon;
				}
				m_selected_ = iNewIndex;
				break;
			}
			if (iNewIndex != -1 && state != kTTSSelect)
			{
				m_tab[iNewIndex].prevstate = m_tab[iNewIndex].state;
				m_tab[iNewIndex].state = state;
			}
		}

		void CTitleTab::AllowNonSelect(bool allow)
		{
			if (m_allow_nselect && !allow)
			{
				if (m_selected_ == -1 && m_tab.size() > 0)
				{
					SetState(0, kTTSSelect);
					//m_selected = 0;
				}
			}
			m_allow_nselect = allow;
		}
	};
};