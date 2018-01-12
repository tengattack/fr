
#ifndef _SNOW_CUTE_VCODE_WINDOW_H_
#define _SNOW_CUTE_VCODE_WINDOW_H_ 1
#pragma once

#include "base/common.h"
#include "base/view.h"
#include "base/frame/frame_window.h"
#include "base/frame/dialog.h"

//control
#include "base/control/button.h"
#include "base/control/check.h"
#include "base/control/text.h"

//base
#include "base/file/file.h"
#include "base/file/filedata.h"
#include "base/lock.h"

#include <net/baidu/BaiduBase.h>

#include "vcode/vcode-win-options.h"
#include "vcode/VerifyCode.h"
#include "vcode/VerifyCodeCharacterSelect.h"

namespace view {
	namespace dlg {

		class CVcodeWindow : public view::frame::CDialog {
		public:
			CVcodeWindow();
			~CVcodeWindow();

			void EnableClose(bool enable_ = true);

			bool AddVcode(TA_REG_QUEUE_INFO *prqi);
			bool DrawPicture(TA_REG_QUEUE_INFO *rqi);
			void DrawSingleVcode(SkCanvas *canvas, int index, CVerifyCode *pvc = NULL);
			void DrawVcode();
			void NextVcode();
			void SkipVcode(DWORD dwVcodeType);
			void SkipFinish(DWORD dwVcodeType);

			void UIUpdateVcode();
			void AdjustCharacterSelect(int x, int y);
			void ShowCharacterSelect(CVerifyCode *pvc);
			inline void HideCharacterSelect() {
				m_char_select->Show(false);
			}
			void CheckCharacterSelect();
			void CharacterSelect(int i);

			void Close();

			void doVcodeAnimation();

      bool IsSkipVcodeType(DWORD dwVcodeType);
			inline bool IsSkipAllVcode() {
				return m_jmpall;
			}

			int WINAPI VcodeChangeCallback(BOOL bSetInfo);
			static LRESULT CALLBACK OnEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

			virtual void JmpVcode();
		//protected:
			virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			static const int kGifVcodeElapse;

		protected:
			bool m_enable_close;
			bool m_hide;
			bool m_first_paint, m_inited;
			bool m_jmpall;
      bool m_first_ocr_show;

			HFONT m_hVcodeFont;

#ifdef _TA_VCODE_USE_SKIA
			SkBitmap m_vcode_list;
#else
			HDC m_hVcodeMemDC;
			HBITMAP m_hVcodeMemBitmap;
#endif

			WNDPROC m_edit_oldWndProc;

			DWORD m_dwInputTimes;

			CVerifyCodeCharacterSelect* m_char_select;

			view::frame::CButton button[1];
			view::frame::CText text[2];

			view::frame::CCheck m_check[1];

			virtual int GetInitialWidth() const
			{
				return 205;	//200
			}
			virtual int GetInitialHeight() const
			{
				return 400;	//370
			}
		};
	}
}

#endif