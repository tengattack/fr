
#ifndef _TA_SNOW_DIALOG_INTERFERENCE_CODE_H_
#define _TA_SNOW_DIALOG_INTERFERENCE_CODE_H_ 1

#include <string>

#include "base/common.h"
#include "base/view.h"
#include "base/frame/dialog.h"

//control
#include "base/control/imagebutton.h"
#include "base/control/text.h"
#include "base/control/combobox.h"

class SkBitmap;

namespace view {
	namespace dlg {

		class CVcodeWindow;
		using namespace ::view::frame;

		class CVerifyCodeCharacterSelect : public CDialog {
		public:
			CVerifyCodeCharacterSelect();
			~CVerifyCodeCharacterSelect();

			bool CreateCharacterSelect(CVcodeWindow *vcode_window);
			bool LoadFriendlyImage(SkBitmap* friendly_image);

			static char* input_str_map[];
			static int id_shift_map[];

		//protected:
			virtual void InitWindow();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			static const int CHAR_COUNT = 9;

		protected:

			static const int CHAR_LINECOUNT = 3;
			static const int CHAR_WIDTH = 60;
			static const int CHAR_HEIGHT = 60;

			CVcodeWindow *m_vcode_window;
			CImageButton m_char_button[CHAR_COUNT];

			virtual int GetInitialWidth() const
			{
				return 180;
			}
			virtual int GetInitialHeight() const
			{
				return 180;
			}

			virtual void OnSize(int width, int height);
		};

	}
}

#endif