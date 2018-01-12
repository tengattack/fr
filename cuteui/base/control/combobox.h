

#ifndef _SNOW_CUTE_COMMBOBOX_H_
#define _SNOW_CUTE_COMMBOBOX_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"

namespace view{
	namespace frame{
		
		class CComboBox : public CChild{

		public:
			CComboBox();
			~CComboBox();

			bool CreateComboBox(HWND hParent, LPCWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST);

/*#define CBS_SIMPLE            0x0001L
#define CBS_DROPDOWN          0x0002L
#define CBS_DROPDOWNLIST      0x0003L
#define CBS_OWNERDRAWFIXED    0x0010L
#define CBS_OWNERDRAWVARIABLE 0x0020L
#define CBS_AUTOHSCROLL       0x0040L
#define CBS_OEMCONVERT        0x0080L
#define CBS_SORT              0x0100L*/

			int AddString(LPCWSTR lpszText);
			int InsertString(int iIndex, LPCWSTR lpszText);
			int DeleteString(int iIndex);
			int FindString(LPCWSTR lpszText, int iIndex = -1);
			int SelectString(LPCWSTR lpszText, int iIndex = -1);

			int Reset();

			int GetCount();
			int GetCurSel();
			int SetCurSel(int iIndex);
		};
	};
};

#endif