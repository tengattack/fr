
#include "stdafx.h"
#include "base/common.h"
#include "base/view.h"
#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_skia.h"
#include "base/frame/dialog.h"

//control
#include "base/control/button.h"
#include "base/control/text.h"

#include "base/operation/fileselect.h"

#include "base/string/stringprintf.h"
#include "base/string/string_number_conversions.h"

#include "base/lock.h"

#include <common/strconv.h>

#include "UserInput.h"

enum _ID_CONTROL{
	//button
	ID_BUTTON_OK = 4001,
	ID_BUTTON_CANCEL,

	//Control
	ID_TEXT_USERNAME,
	ID_TEXT_PASSWORD,

	ID_CHECK_PASSWORD_V,

	//Static
	ID_STATIC_TEXT
};

namespace view {
	namespace dlg {

		CUserInput::CUserInput()
			: m_password_v(false)
		{
		}

		CUserInput::~CUserInput()
		{
		}

		bool CUserInput::doInput(HWND hParent)
		{
			m_ok = false;
			if (CreateDialog(hParent, L"输入账号", true)) {
				DoModal();
			}
			return m_ok;
		}

		void CUserInput::changePasswordVisibility(bool v)
		{
			if (m_password_v == v) {
				return;
			}

			m_password_v = v;

			DWORD style = WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL;
			if (!m_password_v) {
				style |= ES_PASSWORD;
			}

			//SetWindowLong(text[1].hWnd(), GWL_STYLE, style | WS_VISIBLE);

			std::wstring password;
			text[1].GetText(password);
			text[1].Close();

			text[1].CreateText(m_hWnd, password.c_str(), ID_TEXT_PASSWORD, 70, 70, 200, 25, style);
			if (text[0].GetTextLength() > 0 && password.length() > 0) {
				button[0].Enable(true);
			}

			text[1].SetFocus();
		}

		void CUserInput::InitWindow()
		{
			AddStaticText(ID_STATIC_TEXT, 10, 10, 120, 25, L"请输入您的账号:");
			AddStaticText(ID_STATIC_TEXT, 10, 40, 60, 25, L"用户名:");
			AddStaticText(ID_STATIC_TEXT, 10, 70, 60, 25, L"密码:");

			text[0].CreateText(m_hWnd, m_username.c_str(), ID_TEXT_USERNAME, 70, 40, 200, 25);
			text[1].CreateText(m_hWnd, m_password.c_str(), ID_TEXT_PASSWORD, 70, 70, 200, 25, WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_PASSWORD);

			check[0].CreateCheck(m_hWnd, L"密码可见", ID_CHECK_PASSWORD_V, 70, 100, 120, 25);
			if (m_password_v) {
				check[0].SetCheck(true);
			}

			//center
			RECT rect = {0};
			GetClientRect(m_hWnd, &rect);
			int buttonleft = (rect.right - rect.left - 165) / 2;
			if (buttonleft < 0) {
				buttonleft = 0;
			}

			button[0].CreateButton(m_hWnd, L"确定", ID_BUTTON_OK, buttonleft, 140, 80, 25, WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_DEFPUSHBUTTON);
			button[1].CreateButton(m_hWnd, L"取消", ID_BUTTON_CANCEL, buttonleft + 80 + 5, 140, 80, 25);

			button[0].Enable(false);

			text[0].SetFocus();
		}

		LRESULT CALLBACK CUserInput::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_COMMAND:
				
				switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					handled = true;
					switch (LOWORD(wParam))
					{
					case IDOK:
					case ID_BUTTON_OK:
						if (text[0].GetTextLength() > 0 && text[1].GetTextLength() > 0) {
							text[0].GetText(m_username);
							text[1].GetText(m_password);
							m_ok = true;
							Close();
						}
						break;
					case ID_BUTTON_CANCEL:
						Close();
						break;
					case ID_CHECK_PASSWORD_V:
						changePasswordVisibility(check[0].GetCheck());
						break;
					}
					break;
				case EN_CHANGE:
					handled = true;
					switch (LOWORD(wParam))
					{
					case ID_TEXT_USERNAME:
					case ID_TEXT_PASSWORD:
						if (text[0].GetTextLength() > 0 && text[1].GetTextLength() > 0) {
							button[0].Enable(true);
						} else {
							button[0].Enable(false);
						}
						break;
					}
				}
				return 0;
				break;
			}
			return view::frame::CDialog::OnWndProc(message, wParam, lParam, handled);
		}
	}
}