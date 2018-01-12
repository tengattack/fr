

#ifndef _SNOW_CUTE_LINK_H_
#define _SNOW_CUTE_LINK_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "../frame/frame_custom.h"

namespace view {
	namespace frame {
		
		static const wchar_t kLinkClassName[] = L"taLink";

		class CLink : public CCustom {

		public:
			CLink();
			~CLink();

			static const int kFadeStep = 55;
			static const int kFadeElapse = 60;

			enum LinkState {
				kLSNormal = 0,
				kLSHover,
				kLSDown,
				kLSCount
			};

			static bool RegisterClass();

			bool CreateLink(HWND hParent, LPCWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP);

			virtual int OnMouseMove(bool movein);
			virtual int OnMouseLeave();

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
			virtual WindowType GetType();

			virtual void UIUpdate(bool force_ = false);

		protected:
			LinkState m_last_state;
			LinkState m_state;

			void SetState(LinkState state = kLSCount);
			inline bool HaveUnderline(LinkState state) {
				return (m_text_underline[state]);
			}
			inline void SetColor(LinkState state, SkColor color) {
				m_text_color[state] = color;
			}
			inline void SetUnderline(LinkState state, bool underline) {
				m_text_underline[state] = underline;
			}

			SkColor m_text_color[kLSCount];
			bool m_text_underline[kLSCount];

			int m_text_alpha;

		};

	};
};

#endif