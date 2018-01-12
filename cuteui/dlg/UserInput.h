
#ifndef _SNOW_CUTE_DLG_USER_INPUT_H_
#define _SNOW_CUTE_DLG_USER_INPUT_H_ 1
#pragma once

#include "base/common.h"
#include "base/view.h"
#include "base/frame/frame_window.h"
#include "base/frame/dialog.h"

//control
#include "base/control/button.h"
#include "base/control/check.h"
#include "base/control/text.h"

#include <string>

namespace view {
	namespace dlg {

		class CUserInput : public view::frame::CDialog {
		public:
			CUserInput();
			~CUserInput();
			
			bool doInput(HWND hParent);
			void changePasswordVisibility(bool v);

		//protected:
			virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			std::wstring m_username;
			std::wstring m_password;

		protected:
			view::frame::CButton button[2];
			view::frame::CText text[2];
			view::frame::CCheck check[1];

			bool m_ok;
			bool m_password_v;

			virtual int GetInitialWidth() const
			{
				return 300;
			}
			virtual int GetInitialHeight() const
			{
				return 220;
			}
		};
	}
}

#endif