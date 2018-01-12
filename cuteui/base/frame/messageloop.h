
#ifndef _SNOW_CUTE_FRAME_MESSAGELOOP_H_
#define _SNOW_CUTE_FRAME_MESSAGELOOP_H_ 1

#include "../common.h"

namespace view{
	namespace frame{
		namespace messageloop{

			extern int InitLoopThread(int count);
			extern int Loop();

			extern int RunModalLoop(HWND hWnd, HWND hWndParent, DWORD dwFlags);
			extern bool IsIdleMessage(MSG* pMsg);
			extern bool PumpMessage(MSG* pMsg);

		};
	};
};

#endif