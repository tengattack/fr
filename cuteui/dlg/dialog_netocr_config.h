
#ifndef _TA_SNOW_DIALOG_NETOCR_CONFIG_H_
#define _TA_SNOW_DIALOG_NETOCR_CONFIG_H_ 1

#include <string>

#include "base/common.h"
#include "base/view.h"
#include "base/frame/dialog.h"
#include "base/frame/menu.h"

//control
#include "base/control/button.h"
#include "base/control/check.h"
#include "base/control/text.h"
#include "base/control/combobox.h"
#include "base/control/link.h"


namespace view {
	namespace frame {

		class CSnowDialogNetOCRConfig : private CDialog {
		public:
			CSnowDialogNetOCRConfig(HWND hParent);
			~CSnowDialogNetOCRConfig();

			bool doNetOCRConfig();

			virtual void CheckControlEnable();

			virtual void doWork();
      static DWORD WINAPI OCRTestProc(LPVOID lParam);

		//protected:
			virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
			
		protected:

			void LoadConfig();
			void SaveConfig();

			HWND m_wnd_parent;

			bool m_working;
      bool m_bInit;

			CButton m_button[3];
			CText m_text[4];
      CCheck m_check[1];

			CComboBox m_combo[1];
      CLink m_link[1];

			virtual int GetInitialWidth() const
			{
				return 430;
			}
			virtual int GetInitialHeight() const
			{
				return 700;
			}

			//virtual void OnSize(int width, int height);
		};
	}
}

#endif