
#ifndef _SNOW_CUTE_VIEW_GDIPLUS_H_
#define _SNOW_CUTE_VIEW_GDIPLUS_H_ 1

namespace view{
	namespace Gdip{
		void Init();
		void Uninit();

		void MakeRoundRect(RECT* rectangle, int r, GraphicsPath* pPath);
		void MakeRoundRect(RectF* rectangle, int r, GraphicsPath* pPath);

		GpStatus CreateBitmap(GpBitmap **Bitmap, long Width, long Height, PixelFormat pixelformat);
		GpStatus CreateBitmapWithGraphics(GpBitmap **Bitmap, GpGraphics **graphics, long Width, long Height, PixelFormat pixelformat);
		void CreateGlowTextBitmap(Bitmap **TextBmp, Gdiplus::Font *font, FontFamily *fontfamily, FontStyle fontstyle, 
						  float FontSize, StringFormat *strformat,const wchar_t *Text,int cchText,long BmpWidth,long BmpHeight, 
						  Color GlowColor, float GlowRange, float GlowSize, bool DrawInner, 
						  Brush *InnerBrush,float offsetX, float offsetY, bool DrawRange = true);

		BYTE MixAlpha(BYTE fade, BYTE alpha);
	};
};

#endif