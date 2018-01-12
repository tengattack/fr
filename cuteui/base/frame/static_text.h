
#ifndef _SNOW_CUTE_FRAME_STATIC_TEXT_H_
#define _SNOW_CUTE_FRAME_STATIC_TEXT_H_ 1

#include "../common.h"
#include "frame_skia.h"
#include <vector>

namespace view{
	namespace frame{

		typedef struct _TA_STATIC_TEXT {
			unsigned short id;
			bool show;
			POINT point;
			SIZE size;
			std::wstring str;
			int text_size;
		} TA_STATIC_TEXT;

		class CStaticText {
		public:
			CStaticText();
			~CStaticText();

			enum TextStyle {
				kTSLeft = 0,
				kTSRight,
				kTSCenter,
				kTSCount
			};

			virtual int AddStaticText(unsigned short id, int x, int y, int width, int height, LPCWSTR lpszText, bool show_ = true, int text_size = -1);
			virtual void SetStaticText(int index, LPCWSTR lpszText, bool show_ = true);
			virtual void ShowText(unsigned short id, bool show_ = true);
			virtual void SwitchText(unsigned short id);
			
			virtual void OnDrawText(SkCanvas& canvas, SkColor text_color, TextStyle xstyle = kTSLeft, TextStyle ystyle = kTSCenter);

		protected:
			std::vector<TA_STATIC_TEXT> m_static;
			SkScalar m_static_text_size;
		};
	};
};

#endif