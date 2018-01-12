
#ifndef _SNOW_CUTE_VIEW_SKIA_H_
#define _SNOW_CUTE_VIEW_SKIA_H_ 1

#include <skiahelper.h>
#include "basictypes.h"

typedef ICONINFO ScopedICONINFO;

namespace view{
	namespace skia{
		void Init();
		void Uninit();

		class IconUtil {
		public:
			static SkBitmap* CreateSkBitmapFromHICON(HICON icon, int width, int height);
			static SkBitmap* CreateSkBitmapFromHICON(HICON icon);
			static SkBitmap CreateSkBitmapFromHICONHelper(HICON icon, int width, int height);

			static void InitializeBitmapHeader(BITMAPV5HEADER* header, int width, int height);
			static bool PixelsHaveAlpha(const uint32* pixels, size_t num_pixels);
		};
	};
};

#endif