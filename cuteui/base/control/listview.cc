
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "base/frame/frame_theme.h"
#include "listview.h"

namespace view{
	namespace frame{
		

		CListView::CListView()
		{
		}
		CListView::~CListView()
		{
		}

		int CListView::GetSelectedCount()
		{
			return ListView_GetSelectedCount(m_hWnd);
		}

		int CListView::GetFirstSelectedIndex()
		{
			 int count = GetItemCount();
			 for (int i = 0; i < count; i++) {
				 if (IsItemSelected(i)) {
					return i;
				 }
			 }
			 return -1;
		}

		bool CListView::EnsureVisible(int iItem, bool visible)
		{
			return static_cast<bool>(ListView_EnsureVisible(m_hWnd, iItem, visible ? TRUE : FALSE));
		}
		

		HIMAGELIST CListView::GetImageList(int iImageList)
		{
			return ListView_GetImageList(m_hWnd, iImageList);
		}

		HIMAGELIST CListView::SetImageList(int iImageList, HIMAGELIST hImageList)
		{
			return ListView_SetImageList(m_hWnd, hImageList, iImageList);
		}
		
		HWND CListView::GetHeaderCtrl()
		{
			return ListView_GetHeader(m_hWnd);
		}

		int CListView::SetItemCount(int count)
		{
			return ListView_SetItemCount(m_hWnd, count);
		}

		int CListView::GetItemCount()
		{
			return ListView_GetItemCount(m_hWnd);
		}

		bool CListView::SetItemImage(int iItem, int iImage)
		{
			LV_ITEM lvItem = {0};
			lvItem.mask = LVIF_IMAGE;
			lvItem.iItem = iItem;
			if (ListView_GetItem(m_hWnd, &lvItem))
			{
				lvItem.iImage = iImage;
				return ListView_SetItem(m_hWnd, &lvItem);
			}
			return false;
		}
		
		int CListView::SetExtendedStyle(DWORD dwExStyle)
		{
			return ListView_SetExtendedListViewStyle(m_hWnd, dwExStyle);
		}

		DWORD CListView::GetExtendedStyle()
		{
			return ListView_GetExtendedListViewStyle(m_hWnd);
		}

		DWORD CListView::GetItemState(int iItem, UINT mask)
		{
			return ListView_GetItemState(m_hWnd, iItem, mask);
		}

		bool CListView::SetItemState(int iItem, UINT mask, DWORD data)
		{
			//ListView_SetItemState(m_hWnd, iItem, data, mask);
			LV_ITEM _macro_lvi;
			_macro_lvi.stateMask = (mask);
			_macro_lvi.state = (data);
			return SNDMSG((m_hWnd), LVM_SETITEMSTATE, (WPARAM)(iItem), (LPARAM)(LV_ITEM *)&_macro_lvi);
		}

		bool CListView::IsItemSelected(int iItem)
		{
			return GetItemState(iItem, LVIS_SELECTED) & LVIS_SELECTED;
		}

		int CListView::SetItemText(int iItem, int iSubItem, LPCWSTR lpText)
		{
			LV_ITEM lvItem;
			lvItem.iSubItem = iSubItem;
			lvItem.pszText = (LPWSTR)lpText;
			return SendMessageW(m_hWnd, LVM_SETITEMTEXT, (WPARAM)iItem, (LPARAM)(LV_ITEM *)&lvItem);
		}

		int CListView::GetItemText(int iItem, int iSubItem, std::wstring& text)
		{
			wchar_t szText[256];
			LV_ITEM lvItem;
			lvItem.iSubItem = iSubItem;
			lvItem.cchTextMax = 256;
			lvItem.pszText = szText;

			int text_len = SendMessageW(m_hWnd, LVM_GETITEMTEXT, (WPARAM)iItem, (LPARAM)(LV_ITEM *)&lvItem);
			if (text_len > 0)
			{
				text = szText;
			}
			return text_len;
		}

		int CListView::InsertItem(int iItem, LPCWSTR lpText, int iImage)
		{
			LV_ITEM lvItem = {0};
			lvItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
			lvItem.iItem = iItem;
			//lvItem.iSubItem = 0;
			//lvI.pszText = LPSTR_TEXTCALLBACK; //此处触发WM_NOTIFY消息
			lvItem.pszText = (LPWSTR)lpText;
			lvItem.cchTextMax = lstrlenW(lpText) /* MAX_ITEMLEN*/;
			lvItem.iImage = iImage;
			//lvI.iImage = index;
			//lvI.lParam = (LPARAM)&rgHouseInfo[index];
			return ListView_InsertItem(m_hWnd, &lvItem);
		}

		bool CListView::DeleteItem(int iItem)
		{
			return static_cast<bool>(ListView_DeleteItem(m_hWnd, iItem));
		}

		bool CListView::Clear()
		{
			return static_cast<bool>(ListView_DeleteAllItems(m_hWnd));
		}

		int CListView::InsertColumn(int iCol, LPCWSTR lpText, int width)
		{
			LV_COLUMN lvCol = {0};
			lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvCol.fmt = LVCFMT_LEFT;  // left-align column
			lvCol.cx = width;         // width of column in pixels
			lvCol.pszText = (LPWSTR)lpText;
			return ListView_InsertColumn(m_hWnd, iCol, &lvCol);
		}

		int CListView::SetColumn(int iCol, LV_COLUMN* plvCol)
		{
			return ListView_SetColumn(m_hWnd, iCol, plvCol);
		}

		int CListView::GetColumn(int iCol, LV_COLUMN* plvCol)
		{
			return ListView_GetColumn(m_hWnd, iCol, plvCol);
		}

		void CListView::InitWindow()
		{
			//ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TRACKSELECT);   //全行选中,加网格    
			// Focus retangle is not painted properly without double-buffering
		#if (_WIN32_WINNT >= 0x501)
			SetExtendedStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | GetExtendedStyle());
		#else
			SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
		#endif

			theme::EnableWindowTheme(m_hWnd, L"Explorer", L"ListView", NULL);
		}

		bool CListView::CreateListView(HWND hParent, LPWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (CreateEx(hParent, WC_LISTVIEW, lpszText, dwStyle, WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE,
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

		LRESULT CALLBACK CListView::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			return CChild::OnWndProc(message, wParam, lParam, handled);
		}
	};
};