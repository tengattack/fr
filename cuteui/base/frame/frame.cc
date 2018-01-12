
#include "frame.h"
#include "frame_window.h"

#include "../global.h"
#include "../view.h"

#include <map>

namespace view{
	namespace frame{
		std::map<HWND, CWindow*> WindowMap;

		CWindow* GetWindow(HWND hWnd)
		{
			/*std::map<HWND, CWindow*>::iterator iter = WindowMap.begin();
			while (iter != WindowMap.end())
			{
				if (iter->first == hWnd)
				{
					return iter->second;
				}
				iter++;
			}*/

			std::map<HWND, CWindow*>::iterator iter = WindowMap.find(hWnd);
			if (iter != WindowMap.end()) {
				return iter->second;
			}

			return NULL;
		}

		void InsertWindow(HWND hWnd, CWindow* pWindow)
		{
			WindowMap[hWnd] = pWindow;
		}

		void DeleteWindow(CWindow* pWindow)
		{
			std::map<HWND, CWindow*>::iterator iter = WindowMap.begin();
			while (iter != WindowMap.end())
			{
				if (iter->second == pWindow)
				{
					WindowMap.erase(iter);
					break;
				}
				iter++;
			}
		}
	};
};
