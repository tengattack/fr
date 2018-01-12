

#ifndef _SNOW_CUTE_LISTVIEW_H_
#define _SNOW_CUTE_LISTVIEW_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"

namespace view{
	namespace frame{
		
		class CListView : public CChild{
		public:
			CListView();
			~CListView();

			bool CreateListView(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP | LVS_REPORT);
		//protected:
			virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			int InsertColumn(int iCol, LPCWSTR lpText, int width = 50);
			int InsertItem(int iItem, LPCWSTR lpText, int iImage = -1);
			bool DeleteItem(int iItem);
			bool Clear();

			DWORD GetItemState(int iItem, UINT mask);
			bool SetItemState(int iItem, UINT mask, DWORD data);
			bool IsItemSelected(int iItem);

			int SetItemText(int iItem, int iSubItem, LPCWSTR lpText);
			int GetItemText(int iItem, int iSubItem, std::wstring& text);

			bool SetItemImage(int iItem, int iImage);

			int SetExtendedStyle(DWORD dwExStyle);
			DWORD GetExtendedStyle();

			int SetItemCount(int count);
			int GetItemCount();
			int GetSelectedCount();
			int GetFirstSelectedIndex();

			int GetColumn(int iCol, LV_COLUMN* plvCol);
			int SetColumn(int iCol, LV_COLUMN* plvCol);

			bool EnsureVisible(int iItem, bool visible);

			HIMAGELIST SetImageList(int iImageList, HIMAGELIST hImageList);
			HIMAGELIST GetImageList(int iImageList);

			HWND GetHeaderCtrl();
		
		protected:
			//bool m_bHover;
		};
	};
};

#endif