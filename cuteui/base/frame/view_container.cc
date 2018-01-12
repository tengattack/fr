
#include "frame.h"
#include "frame_view.h"
#include "view_container.h"

namespace view{
	namespace frame{
		CViewContainer::CViewContainer(HWND hParent, int x, int y, int width, int height)
		{
			m_wnd_container = hParent;
			SetViewPosition(x, y, width, height);
		}

		CViewContainer::~CViewContainer()
		{
		}

		int CViewContainer::AddView(CView *pView)
		{
			m_view_list.push_back(pView);
			return (m_view_list.size() - 1);
		}

		CView* CViewContainer::GetView(int index)
		{
			if (index >= 0 && index < m_view_list.size()) {
				return m_view_list[index];
			}
			return NULL;
		}

		void CViewContainer::SwitchView(int index)
		{
			for (int i = 0; i < m_view_list.size(); i++)
			{
				if (i == index)
				{
					m_view_list[i]->ShowView(true);
				} else {
					m_view_list[i]->ShowView(false);
				}
			}
		}

		void CViewContainer::SetViewPosition(int x, int y, int width, int height)
		{
			if (x != m_view_x || y != m_view_y || width != m_view_width || height != m_view_height) {
				m_view_x = x;
				m_view_y = y;
				m_view_width = width;
				m_view_height = height;

				for (uint32 i = 0; i < m_view_list.size(); i++)
				{
					m_view_list[i]->SetWindowPos(NULL, 
						m_view_x, m_view_y, m_view_width, m_view_height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
				}
			}
		}

		void CViewContainer::SetViewParent(HWND hParent)
		{
			if (hParent != m_wnd_container) {
				m_wnd_container = hParent;
				for (uint32 i = 0; i < m_view_list.size(); i++)
				{
					::SetParent(m_view_list[i]->hWnd(), m_wnd_container);
					//m_view_list[i]->SetWindowPos(m_wnd_container, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				}
			}
		}
	};
};
