
#include "frame.h"
#include "frame_window.h"

#include "base/global.h"
#include "base/view.h"
#include "resource.h"

#include "static_text.h"

namespace view{
	namespace frame{
		CStaticText::CStaticText()
			: m_static_text_size(13.0f)
		{
		}
		CStaticText::~CStaticText()
		{
		}

		int CStaticText::AddStaticText(unsigned short id, int x, int y, int width, int height, LPCWSTR lpszText, bool show_, int text_size)
		{
			TA_STATIC_TEXT tst;
			//用户列表文件
			tst.id = id;
			tst.show = show_;
			tst.point.x = x;
			tst.point.y = y;
			tst.size.cx = width;
			tst.size.cy = height;
			tst.str = lpszText;
			tst.text_size = text_size;
			m_static.push_back(tst);

			return (m_static.size() - 1);
		}

		void CStaticText::SetStaticText(int index, LPCWSTR lpszText, bool show_)
		{
			if (index >= 0 && index < m_static.size()) {
				if (lpszText) m_static[index].str = lpszText;
				m_static[index].show = show_;
			}
		}

		void CStaticText::ShowText(unsigned short id, bool show_)
		{
			for (unsigned long i = 0; i < m_static.size(); i++)
			{
				if (m_static[i].id == id)
				{
					m_static[i].show = show_;
				}
			}

			//RedrawBackground();
			//Redraw();
		}

		void CStaticText::SwitchText(unsigned short id)
		{
			for (unsigned long i = 0; i < m_static.size(); i++)
			{
				if (m_static[i].id == id)
				{
					m_static[i].show = true;
				} else {
					m_static[i].show = false;
				}
			}

			//RedrawBackground();
			//Redraw();
		}

		void CStaticText::OnDrawText(SkCanvas& canvas, SkColor text_color, TextStyle xstyle, TextStyle ystyle)
		{
			SkPaint paint;

			paint.setColor(text_color);
			paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
			paint.setTypeface(global::skia::font);
			//paint.setTextSize(m_static_text_size);
			paint.setTextAlign(SkPaint::kLeft_Align);
			paint.setAntiAlias(true);

			size_t text_len;

			//画Static Text
			for (uint32 i = 0; i < m_static.size(); i++)
			{
				if (m_static[i].show) {

					if (m_static[i].text_size == -1) {
						paint.setTextSize(m_static_text_size);
					} else {
						paint.setTextSize(m_static[i].text_size);
					}

					SkRect textrect;
					text_len = m_static[i].str.length();
					paint.measureText(m_static[i].str.c_str(), text_len << 1, &textrect);
				
					SkScalar left, top;

					switch (xstyle) {
					case kTSLeft:
						left = m_static[i].point.x - textrect.left();
						break;
					case kTSRight:
						left = m_static[i].point.x - textrect.left() + m_static[i].size.cx - textrect.width();
						break;
					case kTSCenter:
						left = m_static[i].point.x - textrect.left() + (m_static[i].size.cx - textrect.width()) / 2;
						break;
					default:
						left = m_static[i].point.x - textrect.left();
						break;
					}

					switch (ystyle) {
					case kTSLeft:
						top = m_static[i].point.y - textrect.top();
						break;
					case kTSRight:
						top = m_static[i].point.y - textrect.top() + m_static[i].size.cy - textrect.height();
						break;
					case kTSCenter:
						top = m_static[i].point.y - textrect.top() + (m_static[i].size.cy - textrect.height()) / 2;
						break;
					default:
						top = m_static[i].point.y - textrect.top();
						break;
					}

					canvas.drawText(m_static[i].str.c_str(), text_len << 1, 
						left, top, paint);

					/*g.DrawString(m_static[i].str.c_str(), -1, global::Gdip::font, 
						RectF(m_static[i].point.x, m_static[i].point.y, m_static[i].size.cx, m_static[i].size.cy),
						NULL, skin::CSkinData::m_si.main_brush ? skin::CSkinData::m_si.main_brush : global::Gdip::blackbrush);*/
				}
			}
		}

	};
};
