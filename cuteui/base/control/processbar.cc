
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "processbar.h"

#include "base/view_gdiplus.h"
#include "base/view_skia.h"

namespace view{
	namespace frame{
		
		CProcessBar::CProcessBar()
		{
		}
		CProcessBar::~CProcessBar()
		{
		}

		bool CProcessBar::CreateProcessBar(HWND hParent, LPWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (Create(hParent, PROGRESS_CLASS, lpszText, dwStyle,
				x, y,
				width, height, (HMENU)id))
			{
				SetID(id);
				SubClassWindow();

				Show();
				return true;
			} else {
				return false;
			}

			return true;
		}

		int CProcessBar::SetRange(unsigned short min, unsigned short max)
		{
			return SendMessage(m_hWnd, PBM_SETRANGE, 0, MAKELPARAM(min, max));
		}

		int CProcessBar::SetRange32(unsigned long min, unsigned long max)
		{
			return SendMessage(m_hWnd, PBM_SETRANGE32, min, max);
		}

		int CProcessBar::SetStep(unsigned long step)
		{
			return SendMessage(m_hWnd, PBM_SETSTEP, step, 0);
		}

		int CProcessBar::GetStep()
		{
			return SendMessage(m_hWnd, PBM_GETSTEP, 0, 0);
		}

		int CProcessBar::StepIt()
		{
			return SendMessage(m_hWnd, PBM_STEPIT, 0, 0);
		}

		int CProcessBar::SetPos(unsigned int pos)
		{
			return SendMessage(m_hWnd, PBM_SETPOS, pos, 0);
		}

		unsigned int CProcessBar::GetPos()
		{
			return SendMessage(m_hWnd, PBM_GETPOS, 0, 0);
		}
	};
};