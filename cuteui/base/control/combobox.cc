
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "combobox.h"

#include "base/view_gdiplus.h"

namespace view{
	namespace frame{
		
		CComboBox::CComboBox()
		{
		}
		CComboBox::~CComboBox()
		{
		}

		int CComboBox::AddString(LPCWSTR lpszText)
		{
			/*
			Return Value
			The return value is the zero-based index to the string in the list box of the combo box. If an error occurs, the return value is CB_ERR. If insufficient space is available to store the new string, it is CB_ERRSPACE.
			*/
			return SendMessageW(m_hWnd, CB_ADDSTRING, 0, (LPARAM)lpszText);
		}

		int CComboBox::InsertString(int iIndex, LPCWSTR lpszText)
		{
			/*
			wParam
			The zero-based index of the position at which to insert the string. If this parameter is 每1, the string is added to the end of the list.
			
			Return Value
			The return value is the index of the position at which the string was inserted. If an error occurs, the return value is CB_ERR. If there is insufficient space available to store the new string, it is CB_ERRSPACE.
			*/
			return SendMessageW(m_hWnd, CB_INSERTSTRING, iIndex, (LPARAM)lpszText);
		}

		int CComboBox::DeleteString(int iIndex)
		{
			/*
			Return Values
			A count of the strings remaining in the list indicates success. CB_ERR indicates that the index parameter specifies an index greater than the number of items in the list.
			*/
			return SendMessageW(m_hWnd, CB_DELETESTRING, iIndex, 0);
		}

		int CComboBox::GetCount()
		{
			return SendMessageW(m_hWnd, CB_GETCOUNT, 0, 0);
		}

		int CComboBox::GetCurSel()
		{
			return SendMessageW(m_hWnd, CB_GETCURSEL, 0, 0);
		}

		int CComboBox::SetCurSel(int iIndex)
		{
			/*
			wParam
			Specifies the zero-based index of the string to select. If this parameter is 每1, any current selection in the list is removed and the edit control is cleared.			
			
			Return Value
			If the message is successful, the return value is the index of the item selected. If wParam is greater than the number of items in the list or if wParam is 每1, the return value is CB_ERR and the selection is cleared.
			*/
			return SendMessageW(m_hWnd, CB_SETCURSEL, iIndex, 0);
		}

		int CComboBox::FindString(LPCWSTR lpszText, int iIndex)
		{
			/*
			wParam
			The zero-based index of the item preceding the first item to be searched. When the search reaches the bottom of the list box, it continues from the top of the list box back to the item specified by the wParam parameter. If wParam is 每1, the entire list box is searched from the beginning.

			lParam
			A pointer to the null-terminated string that contains the characters for which to search. The search is not case sensitive, so this string can contain any combination of uppercase and lowercase letters.

			Return Value
			The return value is the zero-based index of the matching item. If the search is unsuccessful, it is CB_ERR.
			*/
			return SendMessageW(m_hWnd, CB_FINDSTRING, iIndex, (LPARAM)lpszText);
		}

		int CComboBox::SelectString(LPCWSTR lpszText, int iIndex)
		{
			return SendMessageW(m_hWnd, CB_SELECTSTRING, iIndex, (LPARAM)lpszText);
		}

		int CComboBox::Reset()
		{
			return SendMessageW(m_hWnd, CB_RESETCONTENT, NULL, NULL);
		}
		

		bool CComboBox::CreateComboBox(HWND hParent, LPCWSTR lpszText, unsigned short id, 
			int x, int y, int width, int height, DWORD dwStyle)
		{
			if (Create(hParent, WC_COMBOBOX, lpszText, dwStyle,
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
	};
};