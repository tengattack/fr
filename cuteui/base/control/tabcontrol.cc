
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "tabcontrol.h"

#include "base/view_gdiplus.h"

namespace view{
	namespace frame{
		
		CTabControl::CTabControl()
		{
		}
		CTabControl::~CTabControl()
		{
		}

		HIMAGELIST CTabControl::GetImageList()
		{
			return TabCtrl_GetImageList(m_hWnd);
		}

		HIMAGELIST CTabControl::SetImageList(HIMAGELIST hImageList)
		{
			return TabCtrl_SetImageList(m_hWnd, hImageList);
		}

		int CTabControl::GetItemCount()
		{
			return TabCtrl_GetItemCount(m_hWnd);
		}

		bool CTabControl::GetItem(int iItem, TC_ITEM *pItem)
		{
			return static_cast<bool>(TabCtrl_GetItem(m_hWnd, iItem, pItem));
		}

		bool CTabControl::SetItem(int iItem, TC_ITEM *pItem)
		{
			return static_cast<bool>(TabCtrl_SetItem(m_hWnd, iItem, pItem));
		}

		int CTabControl::InsertItem(int iItem, TC_ITEM *pItem)
		{
			return TabCtrl_InsertItem(m_hWnd, iItem, pItem);
		}

		int CTabControl::InsertItem(int iItem, LPCWSTR lpszText, int iImage)
		{
			TC_ITEM ti = {0};
			ti.mask = TCIF_TEXT;
			ti.pszText = (LPWSTR)lpszText;
			ti.cchTextMax = lstrlenW(lpszText);
			ti.iImage = iImage;
			return InsertItem(iItem, &ti);
		}

		int CTabControl::DeleteItem(int iItem)
		{
			return TabCtrl_DeleteItem(m_hWnd, iItem);
		}

		bool CTabControl::DeleteAllItems()
		{
			return static_cast<bool>(TabCtrl_DeleteAllItems(m_hWnd));
		}

		int CTabControl::GetCurSel()
		{
			return TabCtrl_GetCurSel(m_hWnd);
		}

		int CTabControl::SetCurSel(int iItem)
		{
			return TabCtrl_SetCurSel(m_hWnd, iItem);
		}

		int CTabControl::SetMinTabWidth(int width)
		{
			return TabCtrl_SetMinTabWidth(m_hWnd, width);
		}

		bool CTabControl::CreateTabControl(HWND hParent, LPWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (Create(hParent, WC_TABCONTROL, lpszText, dwStyle,
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

		/*LRESULT CALLBACK CTabControl::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_ERASEBKGND:
				{
					HWND hParent = GetParent();
					HDC hParentDC = (HDC)SendMessage(hParent, WM_TA_GET_MEMORY_HDC, NULL, NULL);
					if (hParentDC)
					{
						handled = true;
						BitBlt((HDC)wParam, 0, 0, 100, 100, hParentDC, 0, 0, SRCCOPY);
						return TRUE;
					}
				}
				break;
			}
			return CChild::OnWndProc(message, wParam, lParam, handled);
		}*/
	};
};