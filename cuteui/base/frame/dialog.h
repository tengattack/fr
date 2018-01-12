
#ifndef _SNOW_CUTE_FRAME_DIALOG_H_
#define _SNOW_CUTE_FRAME_DIALOG_H_ 1

#include "../common.h"
#include "static_text.h"
#include "frame_skia.h"
#include <vector>

namespace view{
	namespace frame{

		static const wchar_t kDialogClassName[] = L"taDialog";

		class CDialog : public CWindow, public CStaticText, public SkiaInterface {
		public:
			CDialog();
			~CDialog();

			enum DialogShowPosition {
				kDSPCenter = 0,
				kDSPRightTop,
				kDSPCount
			};

			static bool RegisterClass();

			virtual void CreateMemoryDC();
			virtual WindowType GetType();

			//virtual void AddStaticText(unsigned short id, int x, int y, int width, int height, LPCWSTR lpszText, bool show_ = true, int text_size = -1);
			virtual void ShowText(unsigned short id, bool show_ = true);
			virtual void SwitchText(unsigned short id);
			virtual void RedrawText();

			virtual void RedrawBackground();
			virtual void Redraw();
			
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			void SendRedrawBackgroundMessage(HWND hWnd);

			bool CreateDialog(HWND hParent, LPCWSTR lpszTitle, bool bIsModal = false, bool bHide = false, DialogShowPosition pos = kDSPCenter);
			bool CreateDialogEx(HWND hParent, LPCWSTR lpszTitle, DWORD dwStyle, bool bIsModal = false, bool bHide = false, DialogShowPosition pos = kDSPCenter);

			int DoModal();
			int MessageLoop();
			
			inline bool IsLooping()
			{
				return m_looping;
			}

		protected:
			
			int m_width;
			int m_height;
			bool m_looping;
			HWND m_parent;

			virtual int GetInitialWidth() const
			{
				return 500;
			}
			virtual int GetInitialHeight() const
			{
				return 320;
			}

			virtual void OnSize(int width, int height);

		public:
			static HICON m_small_icon;
			static HICON m_big_icon;
		};
	};
};

#endif