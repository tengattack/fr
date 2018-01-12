
#ifndef _SNOW_CUTE_FRAME_CHILD_H_
#define _SNOW_CUTE_FRAME_CHILD_H_ 1

#include "../common.h"

class SkBitmap;

namespace view{
	namespace frame{
		class CChild : public CWindow{
		public:
			CChild();
			virtual ~CChild();

			inline unsigned short GetID()
			{
				return m_id;
			}
			inline void SetID(unsigned short id)
			{
				m_id = id;
			}

			virtual int OnMouseMove(bool movein);
			virtual int OnMouseMove(bool movein, int x, int y);
			virtual int OnMouseLeave();

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
		
			SkBitmap* GetParentSkBitmap();

		protected:

			void SubClassWindow();

			WNDPROC m_oldWndProc;
			unsigned short m_id;
			bool m_bHover;
		};
	};
};

#endif