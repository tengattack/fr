
#include "global.h"
#include "common.h"
//#include "resource.h"
#include "view.h"

#include "view_skia.h"

#include "frame/frame.h"
#include "frame/frame_window.h"

#include "scoped_ptr.h"

namespace view{
	namespace skia{
		void Init()
		{
		}

		void Uninit()
		{
		}
		
		SkBitmap* IconUtil::CreateSkBitmapFromHICON(HICON icon, int width, int height) {
		  // We start with validating parameters.
		  if (!icon || !width || !height)
			return NULL;
		  ScopedICONINFO icon_info;
		  if (!::GetIconInfo(icon, &icon_info))
			return NULL;
		  if (!icon_info.fIcon)
			return NULL;
		  return new SkBitmap(CreateSkBitmapFromHICONHelper(icon, width, height));
		}

		SkBitmap* IconUtil::CreateSkBitmapFromHICON(HICON icon) {
		  // We start with validating parameters.
		  if (!icon)
			return NULL;

		  ScopedICONINFO icon_info;
		  BITMAP bitmap_info = { 0 };

		  if (!::GetIconInfo(icon, &icon_info))
			return NULL;

		  if (!::GetObject(icon_info.hbmMask, sizeof(bitmap_info), &bitmap_info))
			return NULL;

		  return new SkBitmap(CreateSkBitmapFromHICONHelper(icon, bitmap_info.bmWidth, bitmap_info.bmHeight));
		}

		SkBitmap IconUtil::CreateSkBitmapFromHICONHelper(HICON icon, int width, int height) {
		  DCHECK(icon);
		  //DCHECK(!s.IsEmpty());

		  // Allocating memory for the SkBitmap object. We are going to create an ARGB
		  // bitmap so we should set the configuration appropriately.
		  SkBitmap bitmap;
		  bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
		  bitmap.allocPixels();
		  bitmap.eraseARGB(0, 0, 0, 0);
		  SkAutoLockPixels bitmap_lock(bitmap);

		  // Now we should create a DIB so that we can use ::DrawIconEx in order to
		  // obtain the icon's image.
		  BITMAPV5HEADER h;
		  InitializeBitmapHeader(&h, width, height);
		  HDC hdc = ::GetDC(NULL);
		  uint32* bits;
		  HBITMAP dib = ::CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&h),
			  DIB_RGB_COLORS, reinterpret_cast<void**>(&bits), NULL, 0);
		  DCHECK(dib);
		  HDC dib_dc = CreateCompatibleDC(hdc);
		  ::ReleaseDC(NULL, hdc);
		  DCHECK(dib_dc);
		  HGDIOBJ old_obj = ::SelectObject(dib_dc, dib);

		  // Windows icons are defined using two different masks. The XOR mask, which
		  // represents the icon image and an AND mask which is a monochrome bitmap
		  // which indicates the transparency of each pixel.
		  //
		  // To make things more complex, the icon image itself can be an ARGB bitmap
		  // and therefore contain an alpha channel which specifies the transparency
		  // for each pixel. Unfortunately, there is no easy way to determine whether
		  // or not a bitmap has an alpha channel and therefore constructing the bitmap
		  // for the icon is nothing but straightforward.
		  //
		  // The idea is to read the AND mask but use it only if we know for sure that
		  // the icon image does not have an alpha channel. The only way to tell if the
		  // bitmap has an alpha channel is by looking through the pixels and checking
		  // whether there are non-zero alpha bytes.
		  //
		  // We start by drawing the AND mask into our DIB.
		  size_t num_pixels = width * height; //s.GetArea();
		  memset(bits, 0, num_pixels * 4);
		  ::DrawIconEx(dib_dc, 0, 0, icon, width, height, 0, NULL, DI_MASK);

		  // Capture boolean opacity. We may not use it if we find out the bitmap has
		  // an alpha channel.
		  scoped_array<bool> opaque(new bool[num_pixels]);
		  for (size_t i = 0; i < num_pixels; ++i)
			opaque[i] = !bits[i];

		  // Then draw the image itself which is really the XOR mask.
		  memset(bits, 0, num_pixels * 4);
		  ::DrawIconEx(dib_dc, 0, 0, icon, width, height, 0, NULL, DI_NORMAL);
		  memcpy(bitmap.getPixels(), static_cast<void*>(bits), num_pixels * 4);

		  // Finding out whether the bitmap has an alpha channel.
		  bool bitmap_has_alpha_channel = PixelsHaveAlpha(
			  static_cast<const uint32*>(bitmap.getPixels()), num_pixels);

		  // If the bitmap does not have an alpha channel, we need to build it using
		  // the previously captured AND mask. Otherwise, we are done.
		  if (!bitmap_has_alpha_channel) {
			uint32* p = static_cast<uint32*>(bitmap.getPixels());
			for (size_t i = 0; i < num_pixels; ++p, ++i) {
			  DCHECK_EQ((*p & 0xff000000), 0u);
			  if (opaque[i])
				*p |= 0xff000000;
			  else
				*p &= 0x00ffffff;
			}
		  }

		  ::SelectObject(dib_dc, old_obj);
		  ::DeleteObject(dib);
		  ::DeleteDC(dib_dc);

		  return bitmap;
		}

		void IconUtil::InitializeBitmapHeader(BITMAPV5HEADER* header, int width, int height)
		{
			  DCHECK(header);
			  memset(header, 0, sizeof(BITMAPV5HEADER));
			  header->bV5Size = sizeof(BITMAPV5HEADER);

			  // Note that icons are created using top-down DIBs so we must negate the
			  // value used for the icon's height.
			  header->bV5Width = width;
			  header->bV5Height = -height;
			  header->bV5Planes = 1;
			  header->bV5Compression = BI_RGB;

			  // Initializing the bitmap format to 32 bit ARGB.
			  header->bV5BitCount = 32;
			  header->bV5RedMask = 0x00FF0000;
			  header->bV5GreenMask = 0x0000FF00;
			  header->bV5BlueMask = 0x000000FF;
			  header->bV5AlphaMask = 0xFF000000;

			  // Use the system color space.  The default value is LCS_CALIBRATED_RGB, which
			  // causes us to crash if we don't specify the approprite gammas, etc.  See
			  // <http://msdn.microsoft.com/en-us/library/ms536531(VS.85).aspx> and
			  // <http://b/1283121>.
			  header->bV5CSType = LCS_WINDOWS_COLOR_SPACE;

			  // Use a valid value for bV5Intent as 0 is not a valid one.
			  // <http://msdn.microsoft.com/en-us/library/dd183381(VS.85).aspx>
			  header->bV5Intent = LCS_GM_IMAGES;
		}

		bool IconUtil::PixelsHaveAlpha(const uint32* pixels, size_t num_pixels)
		{
			for (const uint32* end = pixels + num_pixels; pixels != end; ++pixels) {
			if ((*pixels & 0xff000000) != 0)
				return true;
			}

			return false;
		}
	};
};