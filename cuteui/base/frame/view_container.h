
#ifndef _SNOW_CUTE_FRAME_VIEW_CONTAINER_H_
#define _SNOW_CUTE_FRAME_VIEW_CONTAINER_H_ 1

#include "../common.h"
#include "frame_view.h"
#include <vector>

namespace view{
	namespace frame{
		
		class CViewContainer{
		public:
			friend class CView;

			CViewContainer(HWND hParent, int x, int y, int width, int height);
			virtual ~CViewContainer();

			int AddView(CView *pView);
			CView* GetView(int index);

			virtual void SwitchView(int index);

			void SetViewPosition(int x, int y, int width, int height);
			void SetViewParent(HWND hParent);

		protected:
			
			std::vector<CView *> m_view_list;

			HWND m_wnd_container;
			int m_view_x, m_view_y, m_view_width, m_view_height;
			
		};
	};
};

#endif