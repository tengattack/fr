

#ifndef _SNOW_CUTE_PROCESSBAR_H_
#define _SNOW_CUTE_PROCESSBAR_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"

namespace view{
	namespace frame{
		
		class CProcessBar : public CChild{

		public:
			CProcessBar();
			~CProcessBar();

			int SetRange(unsigned short min, unsigned short max);
			int SetRange32(unsigned long min, unsigned long max);

			int SetStep(unsigned long step);
			int GetStep();
			int StepIt();

			int SetPos(unsigned int pos);
			unsigned int GetPos();

			bool CreateProcessBar(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP);

		};
	};
};

#endif