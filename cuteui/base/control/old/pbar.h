

#ifndef _SNOW_CUTE_PBAR_H_
#define _SNOW_CUTE_PBAR_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "../frame/frame_custom.h"

namespace view{
	namespace frame{
		
		static const wchar_t kPBarClassName[] = L"taProcessBar";

		class CPBar : public CCustom{

		public:
			CPBar();
			~CPBar();

			static bool RegisterClass();

			bool SetRange(unsigned long min, unsigned long max);

			bool SetStep(unsigned long step);
			unsigned long GetStep();
			void StepIt();

			bool SetPos(unsigned long pos);
			unsigned long GetPos();

			bool CreatePBar(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP);

			virtual WindowType GetType();

			virtual void UIUpdate(bool force_ = false);

		protected:
			unsigned long m_min_, m_max_;
			unsigned long m_step_;
			unsigned long m_pos_;
		};

	};
};

#endif