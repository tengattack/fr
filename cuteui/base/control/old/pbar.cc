
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "pbar.h"

#include "base/view_gdiplus.h"
#include "base/view_skia.h"

#include "base/frame/skin_data.h"

namespace view{
	namespace frame{
		
		CPBar::CPBar()
			: m_min_(0)
			, m_max_(100)
			, m_step_(1)
			, m_pos_(0)
		{
		}
		CPBar::~CPBar()
		{
		}

		WindowType CPBar::GetType()
		{
			return kWTPBar;
		}

		bool CPBar::RegisterClass()
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
			wcex.lpszClassName	= kPBarClassName;
			//wcex.hIconSm		= NULL;

			//如果函数失败，返回值为0
			return (RegisterClassEx(&wcex) != 0);
		}

		bool CPBar::CreatePBar(HWND hParent, LPWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (Create(hParent, kPBarClassName, lpszText, dwStyle,
				x, y,
				width, height, (HMENU)id))
			{
				SetID(id);
				
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

		void CPBar::UIUpdate(bool force_)
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
#if 0
				if (hParentMemDC)
				{
					RECT rc, rcParent;
					if (::GetWindowRect(m_hWnd, &rc) && ::GetWindowRect(hParentWnd, &rcParent))
					{
						BitBlt(m_hMemDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
							hParentMemDC, rc.left - rcParent.left, rc.top - rcParent.top, SRCCOPY);
					}
				} else {
					g.Clear(0xffffffff);
				}

				GraphicsPath round;
				RECT rc = {1, 1, m_width-2, m_height-2};
				Gdip::MakeRoundRect(&rc, 2, &round);
				
				LinearGradientBrush lbbg(Gdiplus::Rect(1, 1, m_width-2, m_height-2),
						Color::WhiteSmoke, Color::Transparent,
						(LinearGradientMode)skin::CSkinData::m_pi.background.linear_mode);
				lbbg.SetInterpolationColors(&skin::CSkinData::m_pi.background.colors[0],
					&skin::CSkinData::m_pi.background.pos[0], skin::CSkinData::m_pi.background.n);
				g.FillPath(&lbbg, &round);
#endif

				SkPath round;
				RECT rc = {1, 1, m_width-2, m_height-2};
				SkRect rect = {1, 1, m_width-1, m_height-1};
				//SkiaMakeRoundRect(&rc, 2, &round);

				SkPaint paint;
				SkShader *shader;

				//canvas.clear(SK_ColorWHITE);
				SkPoint p = SkPoint::Make(1, 1);
				SkPoint q = SkPoint::Make(1, m_height - 1);

				SkPoint pts[] ={p, q};
				shader = SkGradientShader::CreateLinear(
					pts, 
					(SkColor *)&skin::CSkinData::m_pi.background.colors[0], &skin::CSkinData::m_pi.background.pos[0], skin::CSkinData::m_pi.background.n,
					SkShader::kMirror_TileMode);
				//pts决定Gradient的起始点和走向，以及过渡完gClors的颜色需要的像素距离
				paint.setShader(shader);
				shader->unref();

				paint.setAntiAlias(true);
				canvas.drawRect(rect, paint);

				if (m_pos_ > m_min_)
				{
					int _poswidth = (int)floor(((float)(m_pos_ - m_min_) * m_width / (m_max_ - m_min_)));
					/*GraphicsPath round2;
					RECT rc = {1, 1, _poswidth-2, m_height-2};
					Gdip::MakeRoundRect(&rc, 2, &round2);

					LinearGradientBrush lbbg(Gdiplus::Rect(1, 1, _poswidth-2, m_height-2),
							Color::WhiteSmoke, Color::Transparent,
							(LinearGradientMode)skin::CSkinData::m_pi.show.linear_mode);
					lbbg.SetInterpolationColors(&skin::CSkinData::m_pi.show.colors[0],
						&skin::CSkinData::m_pi.show.pos[0], skin::CSkinData::m_pi.show.n);
					g.FillPath(&lbbg, &round2);*/

					SkShader *shader2;
					SkPoint p = SkPoint::Make(1, 1);
					SkPoint q = SkPoint::Make(1, _poswidth - 1);

					SkPoint pts[] ={p, q};
					shader2 = SkGradientShader::CreateLinear(
						pts, 
						(SkColor *)&skin::CSkinData::m_pi.show.colors[0], &skin::CSkinData::m_pi.show.pos[0], skin::CSkinData::m_pi.show.n,
						SkShader::kMirror_TileMode);

					SkRect rc = {1, 1, _poswidth-1, m_height-1};
					SkPaint paint;
					paint.setShader(shader2);
					shader->unref();

					paint.setAntiAlias(true);
					canvas.drawRect(rc, paint);
				}

				/*Pen SidePen(skin::CSkinData::m_pi.sidecolor, 1);
				g.DrawPath(&SidePen, &round);*/
				SkPaint pen;
				pen.setColor(skin::CSkinData::m_pi.sidecolor.GetValue());
				pen.setStyle(SkPaint::kStroke_Style);
				pen.setStrokeWidth(1);
				//canvas.drawPath(round, pen);
				//pen.setAntiAlias(true);
				canvas.drawRect(rect, pen);
			}
		}

		//function
		bool CPBar::SetRange(unsigned long min, unsigned long max)
		{
			if (min < max)
			{
				if (m_min_ != min || m_max_ != max)
				{
					m_min_ = min;
					m_max_ = max;
					m_pos_ = min;

					m_need_update = true;
					RePaint();
				}
				return true;
			}
			return false;
		}

		bool CPBar::SetStep(unsigned long step)
		{
			if (step > 0)
			{
				if (m_step_ != step)
				{
					m_step_ = step;
				}
				return true;
			}
			return false;
		}

		unsigned long CPBar::GetStep()
		{
			return m_step_;
		}

		void CPBar::StepIt()
		{
			m_pos_ += m_step_;
			while (m_pos_ > m_max_)
			{
				m_pos_ = m_min_ + (m_pos_ - m_max_);
			}
			m_need_update = true;
			RePaint();
		}

		bool CPBar::SetPos(unsigned long pos)
		{
			if (pos >= m_min_ && pos <= m_max_)
			{
				if (m_pos_ != pos)
				{
					m_pos_ = pos;
					m_need_update = true;
					RePaint();
				}
				return true;
			}
			return false;
		}

		unsigned long CPBar::GetPos()
		{
			return m_pos_;
		}
	};
};