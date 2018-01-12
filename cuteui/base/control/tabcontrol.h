

#ifndef _SNOW_CUTE_TABCONTROL_H_
#define _SNOW_CUTE_TABCONTROL_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"

namespace view{
	namespace frame{
		
		class CTabControl : public CChild{

		public:
			CTabControl();
			~CTabControl();

			//function
			HIMAGELIST GetImageList();
			HIMAGELIST SetImageList(HIMAGELIST hImageList);

			int GetItemCount();

			bool GetItem(int iItem, TC_ITEM *pItem);
			bool SetItem(int iItem, TC_ITEM *pItem);

			int InsertItem(int iItem, TC_ITEM *pItem);
			int InsertItem(int iItem, LPCWSTR lpszText, int iImage = -1);
			int DeleteItem(int iItem);
			bool DeleteAllItems();

			int GetCurSel();
			int SetCurSel(int iItem);

			int SetMinTabWidth(int width);

			//main
			bool CreateTabControl(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP);

			//virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
		};
	};
};

#endif