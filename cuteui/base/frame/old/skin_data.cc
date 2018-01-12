 
#include "frame.h"
#include "frame_window.h"

#include "../global.h"
#include "../view.h"

#include "skin.h"
#include "skin_windowex.h"
#include "skin_data.h"
#include "base/view_gdiplus.h"

#include "base/file/file.h"
#include "base/json/json_reader.h"
#include "base/string/values.h"
#include "base/string/string_number_conversions.h"

#define DELETE_POINTER(p) \
					if (p) { \
						delete p; \
						p = NULL; \
					}

namespace view{
	namespace frame{

		bool LoadStream(IStream** pIStream, void** pResBuffer, LPCWSTR type, const UINT id)
		{
			HINSTANCE hInstance = global::hInstance;
			{
				HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(id), type);
				if(!hResource)
					return false;

				DWORD dwResourceSize = SizeofResource(hInstance, hResource);
				if(!dwResourceSize)
					return false;

				HGLOBAL hResData = LoadResource(hInstance, hResource);
				if(!hResData)
					return false;

				const void* pResourceData = LockResource(hResData);
				if(!pResourceData)
					return false;

				HGLOBAL hResourceBuffer = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
				if(!hResourceBuffer)
				{
					GlobalFree(hResourceBuffer);
					return false;
				}

				void* pResourceBuffer = GlobalLock(hResourceBuffer);
				if(!pResourceBuffer)
				{
					GlobalUnlock(hResourceBuffer);
					GlobalFree(hResourceBuffer);
					return false;
				}

				memcpy(pResourceBuffer, pResourceData, dwResourceSize);
				::FreeResource((HGLOBAL)pResourceData);     //释放内存

				if(!(CreateStreamOnHGlobal(hResourceBuffer, FALSE, pIStream) == S_OK))
				{
					GlobalUnlock(pResourceBuffer);
					GlobalFree(pResourceBuffer);
					return false;
				}

				*pResBuffer = pResourceBuffer;
			}

			return true;
		}

		bool GdipLoadImageFromResource(Image** img, LPCWSTR type, UINT id)
		{
			void* pResBuffer = NULL;
			IStream* pIStream = NULL;
			if(LoadStream(&pIStream, &pResBuffer, type, id))
			{
				*img = new Image(pIStream);
				pIStream->Release();

				GlobalUnlock(pResBuffer);
				GlobalFree(pResBuffer);

				return true;
			}
			return false;
		}

		namespace skin{

			typedef struct _LOAD_INFO_DATA{
				char *name;
				bool (*_LOAD_PROC)(DictionaryValue* v);
			} LOAD_INFO_DATA;

			const static char *kButtonStateText[] = {"normal", "hover", "down", "unenable"};
			const static char *kButtonTypeText[] = {"close", "max", "min"};

			const static LOAD_INFO_DATA lid[] = {
				{"main", CSkinData::LoadMainInfo},
				{"dialog", CSkinData::LoadDialogInfo},
				{"caption", CSkinData::LoadCaptionInfo},
				{"pbar", CSkinData::LoadPBarInfo},
				{"titletab", CSkinData::LoadTitleTabInfo},
				{"button", CSkinData::LoadButtonInfo}
			};

			bool CSkinData::m_enable = false;
			std::wstring CSkinData::m_name;
			std::wstring CSkinData::m_skin_path;
			CAPTION_INFO CSkinData::m_ci = {0};
			BACKGROUND_INFO CSkinData::m_bi = {0};
			STAITC_INFO CSkinData::m_si = {0};
			PBAR_INFO CSkinData::m_pi = {0};
			TITLETAB_INFO CSkinData::m_tti;
			BUTTON_INFO CSkinData::m_btni = {0};
			DIALOG_INFO CSkinData::m_di = {0};

			CSkinData::CSkinData()
			{
			}

			CSkinData::~CSkinData()
			{
			}

			void CSkinData::OnDrawBackground(Graphics *g, CWindow *pWindow, int width, int height)
			{
				/*static const Color backcolors[] = {	
					Color::MakeARGB(255,253,250,252),
					Color::MakeARGB(255,145,208,231),
					Color::MakeARGB(255,155,212,233),
					Color::MakeARGB(255,243,245,248)
				};
				static const float positions[] = { 0, 0.45F, 0.5F, 1.0F };*/
				if (m_bi.usefile)
				{
					g->DrawImage(m_bi.img, -1, -1, width, height);
				}
				else
				{
					LinearGradientBrush lbback(Gdiplus::Rect(-1, -1, width, height),
									Color::WhiteSmoke, Color::Transparent,
									(LinearGradientMode)m_bi.lcolor.linear_mode);
					lbback.SetInterpolationColors(&m_bi.lcolor.colors[0], &m_bi.lcolor.pos[0], m_bi.lcolor.n);
					g->FillRectangle(&lbback, -1, -1, width, height);
				}
			}

			void CSkinData::OnSetCaptionButton(CWindow *pWindow, CCaptionButton cbutton[3])
			{
				// Calculate Caption Button Bounds
				RECT rectScreen;
				::GetWindowRect(pWindow->hWnd(), &rectScreen);
				RECT rect = {rectScreen.left, rectScreen.top, rectScreen.right, rectScreen.bottom};

				SIZE borderSize = {0};
				WindowEx::GetBorderSize(pWindow, &borderSize);
				//rect.Offset(-rect.X, -rect.Y);
				::OffsetRect(&rect, -rect.left, -rect.top);


				RECT buttonRect = {rect.right - borderSize.cx, rect.top + borderSize.cy, 0};
				
				for (int i = 0; i < MAX_CAPTION_BUTTON; i++)
				{
					if (cbutton[i].GetShow())
					{
						buttonRect.left -= (m_ci.cbi[i].width + (i == 0 ? 0 : m_ci.button_spacing));
						buttonRect.right = buttonRect.left + m_ci.cbi[i].width;
						buttonRect.bottom = buttonRect.top + m_ci.cbi[i].height;
						cbutton[i].SetBounds(&buttonRect);
					}
				}

				cbutton[1].SetEnabled(WindowEx::IsDrawMaximizeBox(pWindow));
				cbutton[2].SetEnabled(WindowEx::IsDrawMinimizeBox(pWindow));
			}

			void CSkinData::OnDrawCaptionButton(Graphics *g, CCaptionButton *pbutton)
			{
				CAPTION_BUTTON_TYPE kButtonType = pbutton->GetType();
				if (kButtonType == kCBUnknow) return;

				RECT rc = {0};
				pbutton->GetBounds(&rc);
				int button_width = rc.right - rc.left;
				int button_height = rc.bottom - rc.top;
				
				CAPTION_BUTTON_INFO *pcbi = &m_ci.cbi[kButtonType];
				CAPTION_BUTTON_STATE cbState = kCBNormal;
				int i;
				BYTE alpha;
				//BYTE subalpha;

				if (m_ci.button_draw_usefile)
				{
					ColorMatrix cm = {
							1.0, 0.0, 0.0, 0.0, 0.0,
							0.0, 1.0, 0.0, 0.0, 0.0,
							0.0, 0.0, 1.0, 0.0, 0.0,
							0.0, 0.0, 0.0, 1.0, 0.0,
							0.0, 0.0, 0.0, 0.0, 1.0};

					ImageAttributes ia;
					alpha = (BYTE)pbutton->m_fade;

					if (pbutton->m_fade < 255)
					{
						if (pbutton->GetPressed())
						{
							cbState = kCBHover;
						} else if (pbutton->m_just_pressed) {
							cbState = kCBDown;
						} else if (pbutton->m_just_hovered) {
							cbState = kCBHover;
						} else {
							cbState = kCBNormal;
						}

						//cm.m[3][3] = 1.0f - ((float)alpha / 255) * ((float)alpha / 255);
						cm.m[3][3] = 1.0f - (float)alpha / 255;
						ia.SetColorMatrix(&cm);

						g->DrawImage(m_ci.button_img, RectF(0, 0, (float)pcbi->width, (float)pcbi->height), (float)pcbi->draw[cbState].x, (float)pcbi->draw[cbState].y, (float)pcbi->width, (float)pcbi->height, UnitPixel, &ia);
					}

					if (pbutton->GetPressed())
					{
						cbState = kCBDown;
					} else if (pbutton->GetHovered()) {
						cbState = kCBHover;
					} else {
						cbState = kCBNormal;
					}

					if (!pcbi->draw[cbState].enable) return;

					cm.m[3][3] = (float)alpha / 255;
					ia.SetColorMatrix(&cm);

					g->DrawImage(m_ci.button_img, RectF(0, 0, (float)pcbi->width, (float)pcbi->height), (float)pcbi->draw[cbState].x, (float)pcbi->draw[cbState].y, (float)pcbi->width, (float)pcbi->height, UnitPixel, &ia);
					return;
				}

				//要渐变的颜色 
				Color *fadecolors = NULL;
				if (pbutton->m_fade < 255)
				{
					if (pbutton->GetPressed())
					{
						cbState = kCBHover;
					} else if (pbutton->m_just_pressed) {
						cbState = kCBDown;
					} else if (pbutton->m_just_hovered) {
						cbState = kCBHover;
					} else {
						cbState = kCBNormal;
					}

					if (pcbi->draw[cbState].enable)
					{
						Color *colors = new Color[pcbi->draw[cbState].lcolor.n];
						for (i = 0; i < pcbi->draw[cbState].lcolor.n; i++)
						{
							alpha = Gdip::MixAlpha(pcbi->draw[cbState].lcolor.colors[i].GetA(), (BYTE)(255 - pbutton->m_fade));
							//alpha = (BYTE)((1.0f - ((float)alpha / 255) * ((float)alpha / 255)) * 255);
							/*subalpha = pcbi->draw[cbState].lcolor.colors[i].GetA();
							if (subalpha == 0)
							{
								alpha = 0;
							} else if (subalpha != 255) {
								//非饱和
								alpha = (BYTE)((float)alpha * ((float)subalpha / 255));
							}*/
							colors[i].SetValue((pcbi->draw[cbState].lcolor.colors[i].GetValue() & 0x00ffffff) | ((DWORD)alpha) << Color::AlphaShift);
						}


						//背景Brush 
						LinearGradientBrush lbback(Gdiplus::Rect(0, 0, button_width, button_height),
								Color::WhiteSmoke, Color::Transparent,
								(LinearGradientMode)pcbi->draw[cbState].lcolor.linear_mode);
						lbback.SetInterpolationColors(colors, &pcbi->draw[cbState].lcolor.pos[0], pcbi->draw[cbState].lcolor.n);
						g->FillRectangle(&lbback, 0, 0, button_width, button_height);

						delete[] colors;
					}
				}

				if (pbutton->GetPressed())
				{
					cbState = kCBDown;
				} else if (pbutton->GetHovered()) {
					cbState = kCBHover;
				} else {
					cbState = kCBNormal;
				}

				if (!pcbi->draw[cbState].enable) return;

				fadecolors = new Color[pcbi->draw[cbState].lcolor.n];
				for (i = 0; i < pcbi->draw[cbState].lcolor.n; i++)
				{
					alpha = Gdip::MixAlpha(pcbi->draw[cbState].lcolor.colors[i].GetA(), (BYTE)pbutton->m_fade);
					fadecolors[i].SetValue((pcbi->draw[cbState].lcolor.colors[i].GetValue() & 0x00ffffff) | ((DWORD)alpha) << Color::AlphaShift);
					/*if (cbState == kCBDown)
						::__debugbreak();*/
				}
				//背景Brush 
				LinearGradientBrush lbback(Gdiplus::Rect(0, 0, button_width, button_height),
							Color::WhiteSmoke, Color::Transparent,
							(LinearGradientMode)pcbi->draw[cbState].lcolor.linear_mode);
				lbback.SetInterpolationColors(fadecolors, &pcbi->draw[cbState].lcolor.pos[0], pcbi->draw[cbState].lcolor.n);
				g->FillRectangle(&lbback, 0, 0, button_width, button_height);

				 delete[] fadecolors;
			}

			void CSkinData::OnDrawCaptionButton(Graphics *g, CWindow *pWindow, CCaptionButton cbutton[3], int caption_width, int caption_height)
			{
				const static float positions_dis[] = { 0, 1.0F }; 
				const static Color discolors2[] = {	
					Color::MakeARGB(255,255,255,255),
					Color::MakeARGB(255,91,112,149)
				};
				const static Color discolors3[] = {	
					Color::MakeARGB(255,255,50,50),
					Color::MakeARGB(255,91,112,149)
				};
				const static Color discolors4[] = {	
					Color::MakeARGB(255,205,205,205),
					Color::MakeARGB(255,91,112,149)
				};
				const static Color discolors5[] = {	
					Color::MakeARGB(255,55,50,255),
					Color::MakeARGB(255,91,112,149)
				};

				RECT rcButton;
				int button_width, button_height;
				for (int i = 0; i < MAX_CAPTION_BUTTON; i++)
				{
					if (!cbutton[i].GetShow()) continue;
					continue;

					cbutton[i].GetBounds(&rcButton);

					button_width = rcButton.right-rcButton.left;
					button_height = rcButton.bottom-rcButton.top;

					LinearGradientBrush lbback(Gdiplus::Rect(0, 0, button_width, button_height),
											Color::WhiteSmoke, Color::Transparent,
											LinearGradientModeVertical);
					if (cbutton[i].GetEnabled())
					{
						continue;
					}
					else
					{
						lbback.SetInterpolationColors(discolors4, positions_dis, 2);
					}
					
					g->FillRectangle(&lbback, rcButton.left, rcButton.top - 1, button_width, button_height);
				}
			}

			void CSkinData::OnDrawCaptionText(Graphics *g, CWindow *pWindow, int caption_width, int caption_height)
			{
				std::wstring strCaption;
				int text_len = pWindow->GetText(strCaption);
				if (text_len > 0)
					g->DrawString(strCaption.c_str(), text_len, 
							global::Gdip::font, RectF(25, 0, caption_width, caption_height), 
							global::Gdip::strfmtLC,
							m_ci.text_brush);
			}

			void CSkinData::OnSetRegion(CWindow *pWindow, int width, int height)
			{
				if (pWindow == NULL)
					return;

				// Create a rounded rectangle using Gdi
				HRGN hRegion = ::CreateRoundRectRgn(0, 0, width + 1, height + 1, 9, 9);
				::SetWindowRgn(pWindow->hWnd(), hRegion, TRUE);
				DeleteObject(hRegion);
			}

			void CSkinData::UnloadSkin()
			{
				DELETE_POINTER(m_ci.text_brush)
				DELETE_POINTER(m_bi.img)
				DELETE_POINTER(m_si.main_brush)
				DELETE_POINTER(m_si.dlg_brush)
				DELETE_POINTER(m_ci.button_img)
				DELETE_POINTER(m_pi.sidebrush)
				DELETE_POINTER(m_btni.text_brush)
				DELETE_POINTER(m_tti.select_text_brush)

				ClearLinearColor(&m_bi.lcolor);
				ClearLinearColor(&m_pi.background);
				ClearLinearColor(&m_pi.show);
				ClearLinearColor(&m_di.background);
				m_tti.colors.clear();

				int i;
				for (i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						ClearLinearColor(&m_ci.cbi[i].draw[j].lcolor);
					}
				}
				for (i = 0; i < 4; i++)
				{
					ClearLinearColor(&m_btni.bsi[i].lc);
				}
			}

			void CSkinData::ClearLinearColor(LINEAR_COLOR *lc)
			{
				lc->n = 0;
				lc->pos.clear();
				lc->colors.clear();
			}

			bool CSkinData::LoadLinearColor(DictionaryValue* linearValue, LINEAR_COLOR *lc)
			{
				linearValue->GetInteger("linear_mode", &lc->linear_mode);
				ListValue *lv = NULL;

				if (linearValue->GetList("position", &lv))
				{
					ListValue::iterator iter = lv->begin();
					for (; iter != lv->end(); iter++)
					{
						double data;
						if ((*iter)->GetAsReal(&data))
						{
							lc->pos.push_back(data);
							lc->n++;
						}
					}
				}

				if (linearValue->GetList("colors", &lv))
				{
					ListValue::iterator iter = lv->begin();
					for (; iter != lv->end(); iter++)
					{
						//m_bi.lcolor.n++;
						std::string data;
						if ((*iter)->GetAsString(&data))
						{
							lc->colors.push_back(StringToColor(data));
						}
					}
				}
				return true;
			}

			bool CSkinData::LoadMainInfo(DictionaryValue* v)
			{
				DictionaryValue* vSkinBk = NULL;
				std::string strcolor;
				if (v->GetString("static_color", &strcolor))
				{
					m_si.main_color = StringToColor(strcolor);
					DELETE_POINTER(m_si.main_brush)
					m_si.main_brush = new SolidBrush(m_si.main_color);
				}
				if (v->GetDictionary("background", &vSkinBk))
				{
					vSkinBk->GetBoolean("usefile", &m_bi.usefile);
								
					if (m_bi.usefile)
					{
						//使用背景文件
						vSkinBk->GetString("path", &m_bi.path);
						if (m_bi.path.substr(0, 4).compare(L"res:") == 0)
						{
							//资源文件
							std::wstring strResInfo = m_bi.path.substr(4);
							wchar_t* split = (wchar_t *)_tcsstr(strResInfo.c_str(), L"/");
							if (split)
							{
								split[0] = 0;
								UINT id = 0;
								base::StringToInt(split + 1, NULL, (int *)&id);
								if (id != 0)
								{
									if (m_bi.img)
									{
										delete m_bi.img;
										m_bi.img = NULL;
									}
									GdipLoadImageFromResource(&m_bi.img, strResInfo.c_str(), id);
								}
							}
						} else {
							std::wstring strImgPath = m_skin_path;
							strImgPath += m_bi.path;
									
							if (m_bi.img)
							{
								delete m_bi.img;
								m_bi.img = NULL;
							}
							m_bi.img = new Image(strImgPath.c_str());
						}

					} else {
						LoadLinearColor(vSkinBk, &m_bi.lcolor);
						/*ListValue *lv = NULL;

						vSkinBk->GetInteger("linear_mode", &m_bi.lcolor.linear_mode);

						if (vSkinBk->GetList("position", &lv))
						{
							ListValue::iterator iter = lv->begin();
							for (; iter != lv->end(); iter++)
							{
								double data;
								if ((*iter)->GetAsReal(&data))
								{
									m_bi.lcolor.pos.push_back(data);
									m_bi.lcolor.n++;
								}
							}
						}

						if (vSkinBk->GetList("colors", &lv))
						{
							ListValue::iterator iter = lv->begin();
							for (; iter != lv->end(); iter++)
							{
								//m_bi.lcolor.n++;
								std::string data;
								unsigned long ulData;
								if ((*iter)->GetAsString(&data))
								{
									if (data.length() > 0)
									{
										if (data.c_str()[0] == '#')
										{
											ulData = strtoul(data.c_str() + 1, NULL, 16);
											m_bi.lcolor.colors.push_back(ulData);
										}
									}
								}
							}
						}*/
					}
				}
				return true;
			}

			bool CSkinData::LoadDialogInfo(DictionaryValue* v)
			{
				DictionaryValue *subValue = NULL;
				std::string strcolor;
				if (v->GetString("static_color", &strcolor))
				{
					m_si.dlg_color = StringToColor(strcolor);
					DELETE_POINTER(m_si.dlg_brush)
					m_si.dlg_brush = new SolidBrush(m_si.dlg_color);
				}
				if (v->GetDictionary("background", &subValue))
				{
					LoadLinearColor(subValue, &m_di.background);
				}
				return true;
			}

			bool CSkinData::LoadButtonInfo(DictionaryValue* v)
			{
				DictionaryValue *subValue = NULL;

				std::string data;
				v->GetString("text_color", &data);
				if (data.length() > 0)
				{
					m_btni.text_color = StringToColor(data);
					DELETE_POINTER(m_btni.text_brush)
					m_btni.text_brush = new SolidBrush(m_btni.text_color);
				}

				for (int i = 0; i < 4; i++)
				{
					if (v->GetDictionary(kButtonStateText[i], &subValue))
					{
						std::string data;
						subValue->GetString("side_color", &data);
						m_btni.bsi[i].sidecolor = StringToColor(data);
						LoadLinearColor(subValue, &m_btni.bsi[i].lc);
					}
				}
				return true;
			}

			bool CSkinData::LoadTitleTabInfo(DictionaryValue* v)
			{
				ListValue *lv = NULL;
				m_tti.colors.clear();

				if (v->GetList("colors", &lv))
				{
					ListValue::iterator iter = lv->begin();
					for (; iter != lv->end(); iter++)
					{
						//m_bi.lcolor.n++;
						std::string data;
						if ((*iter)->GetAsString(&data))
						{
							m_tti.colors.push_back(StringToColor(data));
						}
					}
				}

				DictionaryValue* subValue = NULL;
				if (v->GetDictionary("background", &subValue))
				{
					std::string strColor;
					if (subValue->GetString("text_color", &strColor))
					{
						m_tti.background_text_color = StringToColor(strColor);
					}

					LoadLinearColor(subValue, &m_tti.select_lcolor);
				}
				if (v->GetDictionary("select", &subValue))
				{
					std::string strColor;
					if (subValue->GetString("side_color", &strColor))
					{
						m_tti.select_sidecolor = StringToColor(strColor);
					}
					strColor = "";
					if (subValue->GetString("text_color", &strColor))
					{
						m_tti.select_text_color = StringToColor(strColor);
						DELETE_POINTER(m_tti.select_text_brush)
						m_tti.select_text_brush = new SolidBrush(m_tti.select_text_color);
					}
					subValue->GetInteger("alpha", &m_tti.select_alpha);
					if (m_tti.select_alpha < 0)
					{
						m_tti.select_alpha = 0;
					} else if (m_tti.select_alpha > 255){
						m_tti.select_alpha = 255;
					}
					LoadLinearColor(subValue, &m_tti.select_lcolor);
				}
				return (m_tti.colors.size() > 0);
			}

			bool CSkinData::LoadPBarInfo(DictionaryValue* v)
			{
				std::string strColor;
				if (v->GetString("side_color", &strColor))
				{
					m_pi.sidecolor = StringToColor(strColor);
					DELETE_POINTER(m_pi.sidebrush)
					m_pi.sidebrush = new SolidBrush(m_pi.sidecolor);
				}

				DictionaryValue* vPbSub = NULL;
				if (v->GetDictionary("background", &vPbSub))
				{
					LoadLinearColor(vPbSub, &m_pi.background);
				}
				if (v->GetDictionary("show", &vPbSub))
				{
					LoadLinearColor(vPbSub, &m_pi.show);
				}
				return true;
			}

			bool CSkinData::LoadCBInfo(DictionaryValue* cbValue, CAPTION_BUTTON_INFO *cbInfo)
			{
				cbValue->GetInteger("width", &cbInfo->width);
				cbValue->GetInteger("height", &cbInfo->height);

				DictionaryValue* cbDrawValue = NULL;
				DictionaryValue* cbStateValue = NULL;

				if (!cbValue->GetDictionary("draw", &cbDrawValue))
				{
					return false;
				}

				for (int i = 0; i < 3; i++)
				{
					if (cbDrawValue->GetDictionary(kButtonStateText[i], &cbStateValue))
					{
						cbStateValue->GetBoolean("enable", &cbInfo->draw[i].enable);

						if (m_ci.button_draw_usefile)
						{
							//使用文件
							cbStateValue->GetInteger("x", &cbInfo->draw[i].x);
							cbStateValue->GetInteger("y", &cbInfo->draw[i].y);
						} else {
							LoadLinearColor(cbStateValue, &cbInfo->draw[i].lcolor);
						}
					}
				}
				return true;
			}

			bool CSkinData::LoadCaptionInfo(DictionaryValue* v)
			{
				DictionaryValue* vSkinCB = NULL;
				DictionaryValue* vSkinCBParent = NULL;

				v->GetInteger("height", &m_ci.height);
								
				std::string data;
				v->GetString("text_color", &data);
				if (data.length() > 0)
				{
					m_ci.text_color = StringToColor(data);
					DELETE_POINTER(m_ci.text_brush)
					m_ci.text_brush = new SolidBrush(m_ci.text_color);
				}
								
				if (v->GetDictionary("button", &vSkinCBParent))
				{
					vSkinCBParent->GetInteger("spacing", &m_ci.button_spacing);
					vSkinCBParent->GetBoolean("usefile", &m_ci.button_draw_usefile);
					if (m_ci.button_draw_usefile)
					{
						std::wstring imgpath;
						if (vSkinCBParent->GetString("path", &imgpath))
						{
							std::wstring strImgPath = m_skin_path;
							strImgPath += imgpath;
							DELETE_POINTER(m_ci.button_img)
							m_ci.button_img = new Image(strImgPath.c_str());
						}
					}
					for (int i = 0; i < 3; i++)
					{
						if (vSkinCBParent->GetDictionary(kButtonTypeText[i], &vSkinCB))
						{
							LoadCBInfo(vSkinCB, &m_ci.cbi[i]);
						}
					}
				}	//if (vSkinCaption->GetDictionary("button", &vSkinCBParent))

				return true;
			}

			bool CSkinData::LoadSkinFromStream(LPCSTR json_stream)
			{
				std::string json = json_stream;
				DictionaryValue* v = (DictionaryValue*)base::JSONReader::Read(json, false);
				if (!v) return false;

				if (v->GetType() == Value::TYPE_DICTIONARY)
				{
					v->GetBoolean("enable", &m_enable);
					v->GetString("name", &m_name);
						
					DictionaryValue* vSkin = NULL;
					if (v->GetDictionary("skin", &vSkin))
					{
						DictionaryValue* vSkinSub = NULL;

						for (int i = 0; i < sizeof(lid) / sizeof(LOAD_INFO_DATA); i++)
						{
							if (vSkin->GetDictionary(lid[i].name, &vSkinSub))
							{
								lid[i]._LOAD_PROC(vSkinSub);
							}
						}

					}	//if (v->GetDictionary("skin", &vSkin))
						
					delete v;
					return true;
				}

				delete v;
				return false;
			}

			bool CSkinData::LoadSkin(LPCWSTR lpszDirName)
			{
				base::CFile f;
				wchar_t szExePath[MAX_PATH];
				std::wstring skin_path;
				::GetModuleFileNameW(NULL, szExePath, MAX_PATH);
				for (int i = lstrlenW(szExePath) - 1; i > 0; i--)
				{
					if (szExePath[i] == '\\' || szExePath[i] == '/')
					{
						szExePath[i+1] = 0;
						lstrcatW(szExePath, L"skin\\");
						skin_path = szExePath;
						break;
					}
				}

				skin_path += lpszDirName;
				skin_path += L"\\";
				std::wstring json_path = skin_path + L"skin.json";
				m_skin_path = skin_path;

				if (f.Open(base::kFileRead, json_path.c_str()))
				{
					int text_len = f.GetFileSize();
					bool b_ret;
					char *szText = (char *)malloc(text_len + 1);
					if (!szText) return false;
					f.Read((unsigned char *)szText, text_len);
					szText[text_len] = 0;
					b_ret = LoadSkinFromStream(szText);
					free(szText);

					return b_ret;
				}
				return false;
			}

			Color CSkinData::StringToColor(std::string& str)
			{
				if (str.length() > 0)
				{
					if (str.c_str()[0] == '#')
					{
						return strtoul(str.c_str() + 1, NULL, 16);
					}
				}

				return 0;
			}
		};
	};
};
