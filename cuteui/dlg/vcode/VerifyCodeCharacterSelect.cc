
#include "stdafx.h"
//#include "func.h"
#include "base/common.h"
#include "base/view.h"
#include "base/frame/frame_window.h"
#include "base/frame/dialog.h"

#include "base/string/stringprintf.h"

#include <common/strconv.h>

#include "vcode-win-options.h"
#include "../VcodeWindow.h"
#include "VerifyCodeCharacterSelect.h"

enum _ID_CONTROL {
	ID_STATIC_TEXT = 0,
	ID_BUTTON_FIRST = 1000 
};

namespace view {
	namespace dlg {

		using namespace ::view::frame;

		char* CVerifyCodeCharacterSelect::input_str_map[CHAR_COUNT] = {
			"00000000", "00010000", "00020000",
			"00000001", "00010001", "00020001", 
			"00000002", "00010002", "00020002"
		};
		int CVerifyCodeCharacterSelect::id_shift_map[CHAR_COUNT] = {
			7, 8, 9,
			4, 5, 6,
			1, 2, 3,
		};

		CVerifyCodeCharacterSelect::CVerifyCodeCharacterSelect()
			: m_vcode_window(NULL)
		{
		}

		CVerifyCodeCharacterSelect::~CVerifyCodeCharacterSelect()
		{
		}

		bool CVerifyCodeCharacterSelect::CreateCharacterSelect(CVcodeWindow* vcode_window)
		{
			m_vcode_window = vcode_window;
			m_parent = vcode_window->hWnd();
			if (CreateEx(m_parent, kDialogClassName, L"CharacterSelect", 
							 WS_POPUP,
							 WS_EX_CONTROLPARENT,
				0, 0,
				GetInitialWidth(), GetInitialHeight()))
			{
				m_width = GetInitialWidth();
				m_height = GetInitialHeight();

				CreateMemoryDC();
				return true;
			} else {
				return false;
			}

			return true;
		}

		bool CVerifyCodeCharacterSelect::LoadFriendlyImage(SkBitmap* friendly_image)
		{
			if (!friendly_image) {
				return false;
			}

			int left = 0, top = _TA_VCODE_FRIENDLY_CHAR_STARTTOP;

			SkRect dstrect = {0, 0, 60, 60};
			bool isvisible = IsVisible();

			for (int i = 0; i < CHAR_COUNT; i++) {
				SkBitmap bitmap;

				if (friendly_image) {
					bitmap.setConfig(SkBitmap::kARGB_8888_Config, 60, 60);
					bitmap.setIsOpaque(false);
					bitmap.allocPixels();

					SkCanvas canvas(bitmap);
					SkRect srcrect = {left, top, left + _TA_VCODE_FRIENDLY_CHAR_WIDTH, top + _TA_VCODE_FRIENDLY_CHAR_HEIGHT};
					canvas.clear(0x00000000);
					canvas.drawBitmapRectToRect(*friendly_image, &srcrect, dstrect, NULL);
				}

				m_char_button[i].setButtonImage(bitmap, true);
				if (isvisible) {
					m_char_button[i].UIUpdate();
				}

				if (i > 0 && (i + 1) % CHAR_LINECOUNT == 0) {
					left = 0;
					top += _TA_VCODE_FRIENDLY_CHAR_HEIGHT;
				} else {
					left += _TA_VCODE_FRIENDLY_CHAR_WIDTH;
				}
			}
			return true;
		}

		void CVerifyCodeCharacterSelect::InitWindow()
		{
			int left = 0, top = 0;
			for (int i = 0; i < CHAR_COUNT; i++) {
				m_char_button[i].CreateButton(m_hWnd, L"", ID_BUTTON_FIRST + i, left, top, CHAR_WIDTH, CHAR_HEIGHT);
				if (i > 0 && (i + 1) % CHAR_LINECOUNT == 0) {
					left = 0;
					top += CHAR_HEIGHT;
				} else {
					left += CHAR_WIDTH;
				}
			}
		}

		LRESULT CALLBACK CVerifyCodeCharacterSelect::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					WORD id = LOWORD(wParam);
					if (id >= ID_BUTTON_FIRST && id < ID_BUTTON_FIRST + CHAR_COUNT)
					{
						if (m_vcode_window) {
							m_vcode_window->CharacterSelect(id_shift_map[id - ID_BUTTON_FIRST]);
						}
					}
				}
				break;
			}

			return CDialog::OnWndProc(message, wParam, lParam, handled);
		}

		void CVerifyCodeCharacterSelect::OnSize(int width, int height)
		{
			/*if (!IsMinimized()) {
			}*/
		}

	}
}
