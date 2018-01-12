
#include "frame.h"
#include "frame_window.h"
#include "frame_skia.h"

#include "../global.h"
#include "../view.h"

#include "skin.h"
#include "skin_windowex.h"
#include "skin_data.h"

#define CB_TIMER_ELAPSE		60
#define CB_TIMER_STEP		55

namespace view{
	namespace frame{
		namespace skin{
			enum CaptionButtonTimer
			{
				kCBFadeInTimer = 2000,
				kCBFadeOutTimer = 2100,
				kCBFadeDownTimer = 2200
			};

			CSkin::CSkin(CWindow *pWindow)
				: m_pWindow(pWindow)
				, _formIsActive(false)
				, _pressedButton(NULL)
				, _hoveredButton(NULL)
				, m_title_text_size(13)
				, m_icon_bitmap(NULL)
				, m_bHover(false)
				, m_draw_icon(true)
			{
				m_caption_button[0].SetHitTest(HTCLOSE);
				m_caption_button[1].SetHitTest(HTMAXBUTTON);
				m_caption_button[2].SetHitTest(HTMINBUTTON);

				m_icon_dst_rect.set(4, 4, 20, 20);
			}

			CSkin::~CSkin()
			{
				if (m_icon_bitmap) delete m_icon_bitmap;
			}

			LRESULT CALLBACK CSkin::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
			{
				switch (message)
				{
				case WM_TIMER:
					OnTimer(wParam, handled);
					break;
				case WM_NCACTIVATE:
					//重载掉，防止产生一些系统的窗体绘制过程
					handled = true;
					break;
				case WM_KILLFOCUS:
				case WM_SETFOCUS:
				/*case WM_ERASEBKGND:
				case WM_ICONERASEBKGND:*/
				case WM_PAINTICON:
				//case WM_SETTEXT:
					OnNcPaint(true);
					break;
				case WM_STYLECHANGED:
                    UpdateStyle();
					{
						RECT rc = {0};
						::GetWindowRect(m_pWindow->hWnd(), &rc);
						CSkinData::OnSetRegion(m_pWindow, rc.right - rc.left, rc.bottom - rc.top);
					}
                    //_manager.CurrentSkin.OnSetRegion(_parentForm, _parentForm.Size);
                    break;
				case WM_SIZE:
					OnSize(wParam, lParam);
					break;
				case WM_WINDOWPOSCHANGED:
                    if (m_pWindow->IsMaximized())
						::SetWindowRgn(m_pWindow->hWnd(), NULL, TRUE);
                    if ((((WINDOWPOS *)lParam)->flags & SWP_NOSIZE) == 0)
                    {
                        UpdateCaption();
                        OnNcPaint(true);
                    }
                    break;
                case WM_WINDOWPOSCHANGING:
                    /*WINDOWPOS wndPos = (WINDOWPOS)m.GetLParam(typeof(WINDOWPOS));
                    if ((wndPos.flags & (int)SWPFlags.SWP_NOSIZE) == 0)*/
					if ((((WINDOWPOS *)lParam)->flags & SWP_NOSIZE) == 0)
                    {
						CSkinData::OnSetRegion(m_pWindow, ((WINDOWPOS *)lParam)->cx, ((WINDOWPOS *)lParam)->cy);
                        //_manager.CurrentSkin.OnSetRegion(_parentForm, new Size(wndPos.cx, wndPos.cy));
                    }
                    break;
				case WM_SHOWWINDOW:
					{
						RECT rc = {0};
						::GetWindowRect(m_pWindow->hWnd(), &rc);
						CSkinData::OnSetRegion(m_pWindow, rc.right - rc.left, rc.bottom - rc.top);
						OnNcPaint(true);
					}
                    //_manager.CurrentSkin.OnSetRegion(_parentForm, _parentForm.Size);
                    break;

                //#region Handle Form Activation

                case WM_SYSCOMMAND:
                    // redraw
                    if (wParam == SC_RESTORE)
						OnNcPaint(true);
                    break;

                case WM_ACTIVATEAPP:
                    // redraw
                    _formIsActive = (wParam != 0);
                    OnNcPaint(true);
                    break;

                case WM_ACTIVATE:
                    // Set active state and redraw
                    _formIsActive = (WA_ACTIVE == wParam || WA_CLICKACTIVE == wParam);
                    OnNcPaint(true);
                    break;
                case WM_MDIACTIVATE:
                    // set active and redraw on activation 
                    if ((HWND)wParam == m_pWindow->hWnd())
                        _formIsActive = false;
                    else if ((HWND)lParam == m_pWindow->hWnd())
                        _formIsActive = true;
                    OnNcPaint(true);
                    break;
                //#endregion

				case WM_CONTEXTMENU:
					/*
wParam
Handle to the window in which the user right-clicked the mouse. This can be a child window of the window receiving the message. For more information about processing this message, see the Remarks section.
lParam
The low-order word specifies the horizontal position of the cursor, in screen coordinates, at the time of the mouse click.
The high-order word specifies the vertical position of the cursor, in screen coordinates, at the time of the mouse click.
					*/
					{
						handled = true;

						HWND hWnd = (HWND)wParam;
						POINTS pt = MAKEPOINTS(lParam);
						RECT rect, rectClient;
						::GetWindowRect(hWnd, &rect);
						::GetClientRect(hWnd, &rectClient);
						::ClientToScreen(hWnd, (LPPOINT)&rectClient);
						::ClientToScreen(hWnd, &(((LPPOINT)&rectClient)[1]));

						if(pt.x >= rect.left && pt.x <= rect.right &&
							pt.y >= rect.top && pt.y <= rect.bottom &&
							!(pt.x >= rectClient.left && pt.x <= rectClient.right &&
							pt.y >= rectClient.top && pt.y <= rectClient.bottom))	//在窗口非工作区中
						{
							HMENU hMenu = GetSystemMenu(hWnd, FALSE);
							UpdateSystemMenu(hMenu);

							int nID = (int)::TrackPopupMenu(hMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD,
								pt.x, pt.y, NULL, hWnd, NULL);
							if(nID > 0)
							{
								::PostMessage(hWnd, WM_SYSCOMMAND, nID, 0);
							}
						}
					}
					break;
                //#region Handle Mouse Processing
					case WM_NCRBUTTONUP:
						if (wParam == HTCAPTION)
						{
							::SendMessage(m_pWindow->hWnd(), WM_CONTEXTMENU, (WPARAM)m_pWindow->hWnd(), lParam);
						}
						break;
                    // Set Pressed button on mousedown
                    case WM_NCLBUTTONDOWN:
                        OnNcLButtonDown((BYTE)wParam, handled);
                        break;
                    // Set hovered button on mousemove
                    case WM_NCMOUSEMOVE:
                        OnNcMouseMove((BYTE)wParam, handled);
                        break;
                    // perform button actions if a button was clicked
                    case WM_NCLBUTTONUP:
                        // Handle button up
						OnNcLButtonUp((BYTE)wParam, handled);
                        /*if (OnNcLButtonUp(m))
                            handled = true;*/
                        break;
                    // restore button states on mouseleave
                    case WM_NCMOUSELEAVE:
                    case WM_MOUSELEAVE:
                    case WM_MOUSEHOVER:
						m_bHover = false;
                        if (_pressedButton != NULL)
						{
							::KillTimer(m_pWindow->hWnd(), kCBFadeOutTimer + _pressedButton->GetHitTest());
							::KillTimer(m_pWindow->hWnd(), kCBFadeDownTimer + _pressedButton->GetHitTest());

							_pressedButton->m_fade = 0;
                            _pressedButton->SetPressed(false);
							//_pressedButton->m_just_pressed = true;
							if (_hoveredButton == NULL)
								::SetTimer(m_pWindow->hWnd(), kCBFadeInTimer + _pressedButton->GetHitTest(), CB_TIMER_ELAPSE, NULL);
						}
                        if (_hoveredButton != NULL)
                        {
							::KillTimer(m_pWindow->hWnd(), kCBFadeInTimer + _hoveredButton->GetHitTest());

							_hoveredButton->m_fade = 0;
							_hoveredButton->SetHovered(false);
							::SetTimer(m_pWindow->hWnd(), kCBFadeOutTimer + _hoveredButton->GetHitTest(), CB_TIMER_ELAPSE, NULL);

                            _hoveredButton = NULL;
                        }
                        //OnNcPaint(true);
                        break;
                //#endregion

				//Non Client Area
				case WM_NCCALCSIZE:
					if (wParam)
					{
						handled = true;
						NCCALCSIZE_PARAMS *ncp = (NCCALCSIZE_PARAMS *)lParam;
						ncp->rgrc[0].top += WindowEx::GetCaptionHeight(m_pWindow);
					}
					break;
				case WM_NCPAINT:
					//handled = true;
					{
						handled = OnNcPaint(true);
					}
					break;
                case WM_NCHITTEST:
					{
                        int lResult = OnNcHitTest(((POINTS FAR *)&(lParam)), handled);
						if (handled)
							return lResult;
					}
                    break;
				}

				return TRUE;
			}

			bool CSkin::OnNcPaint(bool invalidateBuffer)
			{
				bool result = false;

				HDC hdc = NULL;
				HRGN hrgn = NULL;

				//try
				{
					// no drawing needed
					/*if (_parentForm.MdiParent != null && _parentForm.WindowState == FormWindowState.Maximized)
					{
						_currentCacheSize = Size.Empty;
						return false;
					}*/

					// prepare image bounds
					SIZE borderSize = {0};
					WindowEx::GetBorderSize(m_pWindow, &borderSize);
					int captionHeight = WindowEx::GetCaptionHeight(m_pWindow);

					if (m_draw_icon && (m_icon_dst_rect.top() +  m_icon_dst_rect.height()) > captionHeight) {
						captionHeight = m_icon_dst_rect.top() +  m_icon_dst_rect.height();
					}

					RECT rectScreen;
					GetWindowRect(m_pWindow->hWnd(), &rectScreen);

					int wndWidth = rectScreen.right - rectScreen.left;

					// create graphics handle
					hdc = ::GetDCEx(m_pWindow->hWnd(), 0,
						(DCX_CACHE | DCX_CLIPSIBLINGS | DCX_WINDOW));

					SkBitmap skskin;
					skskin.setConfig(SkBitmap::kARGB_8888_Config, wndWidth, captionHeight);
					skskin.allocPixels();
					//skskin.setIsOpaque(true);

					SkCanvas canvas(skskin);

					//画背景
					SkBitmap *parentbitmap = (SkBitmap *)SendMessage(m_pWindow->hWnd(), WM_TA_GET_SKBITMAP, NULL, NULL);
					if (parentbitmap) {
						SkRect drawrect = {0, 0, wndWidth, captionHeight};
						canvas.drawBitmapRectToRect(*parentbitmap, &drawrect, drawrect, NULL);
					} else {
						const static float positions_dis[] = { 0, 1.0F }; 
						const static Color discolors[] = {  
							Color::MakeARGB(255,107,126,160),
							Color::MakeARGB(255,91,112,149)
						};
						SkPaint paint;
						SkShader *shader;
						SkRect rect = {0, 0, rectScreen.right-rectScreen.left, captionHeight};

						//canvas.clear(SK_ColorWHITE);
						SkPoint p = SkPoint::Make(0, 0);
						SkPoint q = SkPoint::Make(0, captionHeight);

						SkPoint pts[] ={p, q};
						shader = SkGradientShader::CreateLinear(
							pts, 
							(SkColor *)discolors, positions_dis, 2,
							SkShader::kMirror_TileMode);
						//pts决定Gradient的起始点和走向，以及过渡完gClors的颜色需要的像素距离
						paint.setShader(shader);
						shader->unref();

						paint.setAntiAlias(true);
						canvas.drawRect(rect, paint);
					}

					if (m_draw_icon) {
						//画图标
						HICON hIcon = (HICON)SendMessage(m_pWindow->hWnd(), WM_GETICON, ((m_icon_dst_rect.height() > 16) ? ICON_BIG : ICON_SMALL), NULL);
						if (hIcon)
						{
							if (!m_icon_bitmap) m_icon_bitmap = view::skia::IconUtil::CreateSkBitmapFromHICON(hIcon);
							if (m_icon_bitmap) {
								SkRect srcrect = {0, 0, m_icon_bitmap->width(), m_icon_bitmap->height()};
								canvas.drawBitmapRectToRect(*m_icon_bitmap, &srcrect, m_icon_dst_rect, NULL);
							}
							//::DrawIconEx(m_hSkinMemDC, 4, 4, hSmall, 16, 16, NULL, NULL, DI_NORMAL);
						}
					}

					//画标题文本
					CSkinData::OnDrawCaptionText(canvas, m_pWindow, rectScreen.right-rectScreen.left, captionHeight, m_title_text_size);

					//画标题按钮
					//CSkinData::OnDrawCaptionButton(g, m_pWindow, m_caption_button, rectScreen.right-rectScreen.left, captionHeight);
					for (int i = 0; i < MAX_CAPTION_BUTTON; i++)
					{
						OnCaptionButtonPaint(&m_caption_button[i], &canvas);
					}

					//BitBlt(hdc, 0, 0, rectScreen.right-rectScreen.left, captionHeight, m_hSkinMemDC, 0, 0, SRCCOPY);
					SkiaPaint(hdc, 0, 0, rectScreen.right-rectScreen.left, captionHeight, skskin, 0, 0);

					result = true;
				}
				/*catch (...)
				{// error drawing
					result = false;
				}*/

				// cleanup data
				if (hdc != NULL)
				{
					//::SelectClipRgn(hdc, NULL);
					::ReleaseDC(m_pWindow->hWnd(), hdc);
				}
				/*if (region != null && hrgn != (IntPtr)0)
					region.ReleaseHrgn(hrgn);*/

				return result;
			}

			int CSkin::OnNcHitTest(POINTS* lpPoint, bool& handled)
			{
				/*if (!IsProcessNcArea)
					return false;*/

				RECT rectScreen;
				WindowEx::GetScreenRect(m_pWindow, &rectScreen);
				Rect rect = Rect(rectScreen.left, rectScreen.top, rectScreen.right - rectScreen.left, rectScreen.bottom - rectScreen.top);

				// custom processing
				if (rect.Contains(lpPoint->x, lpPoint->y))
				{
					SIZE borderSize = {0};
					WindowEx::GetBorderSize(m_pWindow, &borderSize);
					rect.Inflate(-borderSize.cx, -borderSize.cy);

					// let form handle hittest itself if we are on borders
					if (!rect.Contains(lpPoint->x, lpPoint->y))
						return 0;

					Rect rectCaption = rect;
					rectCaption.Height = WindowEx::GetCaptionHeight(m_pWindow);

					// not in caption -> client
					if (!rectCaption.Contains(lpPoint->x, lpPoint->y))
					{
						handled = true;
						if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
						{
							//鼠标左键允许在客户区拖动
							return HTCAPTION;
						} else {
							return HTCLIENT;
						}
					}

					// on icon?
					if (WindowEx::HasMenu(m_pWindow))
					{
						Rect rectSysMenu = rectCaption;
						//SystemInformation.SmallIconSize;
						rectSysMenu.Width = 16;
						rectSysMenu.Height = 16;
						if (rectSysMenu.Contains(lpPoint->x, lpPoint->y))
						{
							handled = true;
							return HTSYSMENU;
						}
					}

					// on Button?
					POINT pt = {lpPoint->x - rectScreen.left, lpPoint->y - rectScreen.top};
					CCaptionButton *sysButton = CommandButtonFromPoint(&pt);
					if (sysButton != NULL)
					{
						handled = true;
						return (int)sysButton->GetHitTest();
					}

					// on Caption?
					handled = true;
					return HTCAPTION;
				}
				handled = true;
				return HTNOWHERE;
			}

			void CSkin::OnNcLButtonUp(BYTE hittest, bool& handled)
			{
				/*if (!IsProcessNcArea)
					return false;*/

				// do we have a pressed button?
				if (_pressedButton != NULL)
				{
					// get button at wparam
					CCaptionButton *button = CommandButtonByHitTest(hittest);
					if (button == NULL)
					{
						//handled = false;
						return;
					}

					if (button->GetPressed())
					{
						switch (button->GetHitTest())
						{
							case HTCLOSE:
								if (m_caption_button[0].GetEnabled())
								{
									m_pWindow->Close();
									handled = true;

									return;
								}
							case HTMAXBUTTON:
								/*if (_parentForm.WindowState == FormWindowState.Maximized)
								{
									_parentForm.WindowState = FormWindowState.Normal;
									_parentForm.Refresh();
								}
								else if (_parentForm.WindowState == FormWindowState.Normal ||
										 _parentForm.WindowState == FormWindowState.Minimized)
								{
									_parentForm.WindowState = FormWindowState.Maximized;
								}*/
								if (m_caption_button[1].GetEnabled())
								{
									SendMessage(m_pWindow->hWnd(), WM_SYSCOMMAND, SC_MAXIMIZE, NULL);
								}
								break;
							case HTMINBUTTON:
								/*_parentForm.WindowState = _parentForm.WindowState == FormWindowState.Minimized
															  ? FormWindowState.Normal
															  : FormWindowState.Minimized;*/
								if (m_caption_button[2].GetEnabled())
								{
									SendMessage(m_pWindow->hWnd(), WM_SYSCOMMAND, SC_MINIMIZE, NULL);
								}
								break;

						}
					}

					::KillTimer(m_pWindow->hWnd(), kCBFadeOutTimer + button->GetHitTest());
					::KillTimer(m_pWindow->hWnd(), kCBFadeDownTimer + button->GetHitTest());

					button->m_fade = 0;
					button->SetPressed(false);
					//_pressedButton->SetHovered(false);
					//_pressedButton->m_just_pressed = true;
					::SetTimer(m_pWindow->hWnd(), kCBFadeInTimer + button->GetHitTest(), CB_TIMER_ELAPSE, NULL);

					_pressedButton = NULL;
				}
				//return false;
			}

			void CSkin::OnNcLButtonDown(BYTE hittest, bool& handled)
			{
				CCaptionButton *button = CommandButtonByHitTest(hittest);
				if (_pressedButton != button && _pressedButton != NULL)
					_pressedButton->SetPressed(false);
				if (button != NULL)
				{
					::KillTimer(m_pWindow->hWnd(), kCBFadeOutTimer + button->GetHitTest());
					::KillTimer(m_pWindow->hWnd(), kCBFadeInTimer + button->GetHitTest());

					button->m_fade = 0;
					::SetTimer(m_pWindow->hWnd(), kCBFadeDownTimer + button->GetHitTest(), CB_TIMER_ELAPSE, NULL);
					button->SetPressed(true);
				}
				_pressedButton = button;
				if (_pressedButton != NULL)
				{
					handled = true;
					//return TRUE;
				}
			}

			void CSkin::OnNcMouseMove(BYTE hittest, bool& handled)
			{
				// Check for hovered and pressed buttons
				//if (Control.MouseButtons != MouseButtons.Left)
				bool movein = !m_bHover;
				m_bHover = true;

				if (movein)
				{
					TRACKMOUSEEVENT tme;
					tme.cbSize = sizeof(tme);
					tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
					tme.hwndTrack = m_pWindow->hWnd();
					tme.dwHoverTime = 0;	//HOVER_DEFAULT
					TrackMouseEvent(&tme);
				}

				bool bpaint = false;
				if (_pressedButton != NULL)
				{
					if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
					{
						_pressedButton->SetPressed(false);
						_pressedButton = NULL;
					}
				}
				CCaptionButton *button = CommandButtonByHitTest(hittest);

				if (_hoveredButton != button && _hoveredButton != NULL)
				{
					if (_hoveredButton->GetHovered())
					{
						bpaint = true;
						//leave

						::KillTimer(m_pWindow->hWnd(), kCBFadeInTimer + _hoveredButton->GetHitTest());

						_hoveredButton->SetHovered(false);
						_hoveredButton->m_fade = 0;
						::SetTimer(m_pWindow->hWnd(), kCBFadeOutTimer + _hoveredButton->GetHitTest(), CB_TIMER_ELAPSE, NULL);
					}
				}
				if (_pressedButton == NULL)
				{
					if (button != NULL)
					{
						button->SetHovered(true);
					}
					if (_hoveredButton != button)
					{
						if (button)
						{
							::KillTimer(m_pWindow->hWnd(), kCBFadeOutTimer + button->GetHitTest());
							
							button->m_fade = 0;
							::SetTimer(m_pWindow->hWnd(), kCBFadeInTimer + button->GetHitTest(), CB_TIMER_ELAPSE, NULL);
						}
						bpaint = true;
					}
					_hoveredButton = button;
				}
				else
				{
					if (button != _pressedButton && _pressedButton->GetPressed())
						_pressedButton->SetPressed(false);
				}

				//if (bpaint) OnNcPaint(true);
			}

			void CSkin::OnSize(WPARAM wParam, LPARAM lParam)
			{
				UpdateCaption();
				// update form styles on maximize/restore
				/*if (_parentForm.MdiParent != null)
				{
					if ((int)m.WParam == 0)
						UpdateStyle();
					if ((int)m.WParam == 2)
						_parentForm.Refresh();
				}*/

				// update region if needed
				//bool wasMaxMin = (m_pWindow->IsMaximized() || m_pWindow->IsMinimized());

				RECT rc = {0};
				::GetWindowRect(m_pWindow->hWnd(), &rc);

				/*if (wasMaxMin && _parentForm.WindowState == FormWindowState.Normal &&
					rc.Size == _parentForm.RestoreBounds.Size)*/
				{
					CSkinData::OnSetRegion(m_pWindow, rc.right - rc.left, rc.bottom - rc.top);
					OnNcPaint(true);
				}
			}

			void CSkin::OnTimer(int nIDEvent, bool& handled)
			{
				int hittest = 0;
				CCaptionButton* cb = NULL;
				
				if ((hittest = nIDEvent - kCBFadeDownTimer) > 0)
				{
					goto dopaint_l;
				} else if ((hittest = nIDEvent - kCBFadeOutTimer) > 0){
					goto dopaint_l;
				} else if ((hittest = nIDEvent - kCBFadeInTimer) > 0){
					goto dopaint_l;
				}

			dopaint_l:
				if (hittest > 0)
				{
					handled = true;
					cb = CommandButtonByHitTest(hittest);
					if (cb)
					{
						cb->m_fade += CB_TIMER_STEP;
						if (cb->m_fade >= 255)
						{
							cb->m_fade = 255;
							::KillTimer(m_pWindow->hWnd(), nIDEvent);
						}

						OnCaptionButtonPaint(cb);
					}
				}
			}

			void CSkin::OnCaptionButtonPaint(CCaptionButton* button, SkCanvas *pCanvas)
			{
				if (!button->GetShow()) return;

				SkCanvas *ptr_canvas = NULL;
				SkBitmap bitmap;

				RECT rc = {0};
				button->GetBounds(&rc);
				int button_width = rc.right - rc.left;
				int button_height = rc.bottom - rc.top;

				if (pCanvas) {
					ptr_canvas = pCanvas;
				} else {
					bitmap.setConfig(SkBitmap::kARGB_8888_Config, button_width, button_height);
					bitmap.allocPixels();

					ptr_canvas = new SkCanvas(bitmap);
				}
				
				if (ptr_canvas) {
					
					SkRect srcrect = {rc.left, rc.top, rc.left + button_width, rc.top + button_height};

					//画背景
					SkBitmap *parent_bitmap = (SkBitmap *)SendMessage(m_pWindow->hWnd(), WM_TA_GET_SKBITMAP, NULL, NULL);
					if (parent_bitmap) {
						SkRect dstrect = {0, 0, button_width, button_height};
						if (pCanvas) {
							dstrect = srcrect;
						}
						
						ptr_canvas->drawBitmapRectToRect(*parent_bitmap, &srcrect, dstrect, NULL);
					} else {
						ptr_canvas->clear(Color::MakeARGB(255,107,126,160));
					}

					CSkinData::OnDrawCaptionButton(*ptr_canvas, button, pCanvas ? &srcrect : NULL);

					if (!pCanvas) {
						HDC hdc = ::GetDCEx(m_pWindow->hWnd(), 0,
							(DCX_CACHE | DCX_CLIPSIBLINGS | DCX_WINDOW));

						SkiaPaint(hdc, rc.left, rc.top, button_width, button_height, bitmap, 0, 0);
						
						ReleaseDC(m_pWindow->hWnd(), hdc);

						delete ptr_canvas;
					}
				}
			}

			void CSkin::UpdateStyle()
			{
				// remove the border style
				int currentStyle = ::GetWindowLong(m_pWindow->hWnd(), GWL_STYLE);
				if (currentStyle & WS_BORDER)
				{
					currentStyle &= ~WS_BORDER;
					
					::SetWindowLong(m_pWindow->hWnd(), GWL_STYLE, currentStyle);

					//OnNcPaint(true);
				}

				::SetClassLong(m_pWindow->hWnd(), GCL_STYLE, GetClassLong(m_pWindow->hWnd(), GCL_STYLE) | CS_DROPSHADOW);

				::SetWindowPos(m_pWindow->hWnd(), NULL, -1, -1, -1, -1,
										SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE |
												SWP_FRAMECHANGED | SWP_NOREDRAW | SWP_NOACTIVATE);
			}

			void CSkin::UpdateCaption()
			{
				CSkinData::OnSetCaptionButton(m_pWindow, m_caption_button);
			}

			CCaptionButton* CSkin::CommandButtonByHitTest(BYTE hittest)
			{
				switch (hittest)
				{
				case HTCLOSE:
					if (m_caption_button[0].GetShow()) return &m_caption_button[0];
					break;
				case HTMAXBUTTON:
					if (m_caption_button[1].GetShow()) return &m_caption_button[1];
					break;
				case HTMINBUTTON:
					if (m_caption_button[2].GetShow()) return &m_caption_button[2];
					break;
				}
				return NULL;
			}

			CCaptionButton* CSkin::CommandButtonFromPoint(LPPOINT lpPoint)
			{
				RECT rc;
				for (int i = 0; i < MAX_CAPTION_BUTTON; i++)
				{
					if (!m_caption_button[i].GetShow()) continue;
					m_caption_button[i].GetBounds(&rc);
					if (lpPoint->x >= rc.left && lpPoint->y >= rc.top &&
						lpPoint->x <= rc.right && lpPoint->y <= rc.bottom)
						return &m_caption_button[i];
				}
				return NULL;
			}

			CCaptionButton* CSkin::GetCaptionButton(int index)
			{
				if (index >= 0 && index < MAX_CAPTION_BUTTON) {
					return &m_caption_button[index];
				}
				return NULL;
			}

			void CSkin::UpdateSystemMenu(HMENU hMenu)
			{
				typedef struct _MENU_ITEM_STATE {
					UINT nCommand;
					UINT nState;
				} MENU_ITEM_STATE;

				const static MENU_ITEM_STATE MIState[]={
					{SC_RESTORE, MFS_GRAYED},
					{SC_MOVE, MFS_ENABLED},
					{SC_SIZE, MFS_GRAYED},
					{SC_MINIMIZE, MFS_ENABLED},
					{SC_MAXIMIZE, MFS_GRAYED}
				};

				for (int i = 0; i < sizeof(MIState) / sizeof(MENU_ITEM_STATE); i++)
				{
					::EnableMenuItem(hMenu, MIState[i].nCommand, MIState[i].nState | MF_BYCOMMAND);
				}

				::SetMenuDefaultItem(hMenu, SC_CLOSE, MF_BYCOMMAND);

			}

			void CSkin::SetIcon(SkBitmap *icon_bitmap, SkBitmap::Config config)
			{
				if (icon_bitmap) {
					if (!m_icon_bitmap) {
						m_icon_bitmap = new SkBitmap;
					}
					icon_bitmap->copyTo(m_icon_bitmap, config);
				} else {
					if (m_icon_bitmap) {
						delete m_icon_bitmap;
						m_icon_bitmap = NULL;
					}
				}
			}
		};
	};
};
