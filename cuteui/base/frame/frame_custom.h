
#ifndef _SNOW_CUTE_FRAME_CUSTOM_H_
#define _SNOW_CUTE_FRAME_CUSTOM_H_ 1

#include "../common.h"
#include "frame_child.h"
#include "frame_skia.h"

namespace view{
	namespace frame{
		class CCustom : public CChild, public SkiaInterface{
		public:
			CCustom();
			~CCustom();

			virtual bool PaintParent(SkCanvas& canvas);

			virtual void RePaint();
			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);

			virtual void OnSize(int width, int height);

		protected:
			int m_width;
			int m_height;
			bool m_need_update;

			virtual void CreateMemoryDC();
			virtual void UIUpdate(bool force_ = false);
		};
	};
};

#endif