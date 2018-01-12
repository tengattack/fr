
#include "global.h"
#include "common.h"
//#include "resource.h"
#include "view.h"

#include "view_gdiplus.h"

#include "frame/frame.h"
#include "frame/frame_window.h"


namespace view{
	namespace Gdip{

		static GdiplusStartupInput m_gdiplusStartupInput;
		static ULONG_PTR m_pGdiToken = NULL;

		void Init()
		{
			GdiplusStartup(&m_pGdiToken ,&m_gdiplusStartupInput, NULL);
		}

		void Uninit()
		{
			GdiplusShutdown(m_pGdiToken);
		}

		void MakeRoundRect(RECT* rectangle, int r, GraphicsPath* pPath)
		{	
			if (!pPath)
				return;
			int l = 2 * r;
			pPath->AddLine(rectangle->left + r, rectangle->top, rectangle->right - r, rectangle->top);
            pPath->AddArc(rectangle->right - l, rectangle->top, l, l, 270.0f, 90.0f);
            pPath->AddLine(rectangle->right, rectangle->top + r, rectangle->right, rectangle->bottom - r);
            pPath->AddArc(rectangle->right - l, rectangle->bottom - l, l, l, 0.0f, 90.0f);
            pPath->AddLine(rectangle->right - r, rectangle->bottom, rectangle->left + r, rectangle->bottom);
            pPath->AddArc(rectangle->left, rectangle->bottom - l, l, l, 90.0f, 90.0f);
            pPath->AddLine(rectangle->left, rectangle->bottom - r, rectangle->left, rectangle->top + r);
            pPath->AddArc(rectangle->left, rectangle->top, l, l, 180.0f, 90.0f);

			return;
		}

		void MakeRoundRect(RectF* rectangle, int r, GraphicsPath* pPath)
		{	
			if (!pPath)
				return;
			int l = 2 * r;
			pPath->AddLine(rectangle->X + r, rectangle->Y, rectangle->GetRight() - r, rectangle->Y);
            pPath->AddArc(rectangle->GetRight() - l, rectangle->Y, (float)l, (float)l, 270.0f, 90.0f);
            pPath->AddLine(rectangle->GetRight(), rectangle->Y + r, rectangle->GetRight(), rectangle->GetBottom() - r);
            pPath->AddArc(rectangle->GetRight() - l, rectangle->GetBottom() - l, (float)l, (float)l, 0.0f, 90.0f);
            pPath->AddLine(rectangle->GetRight() - r, rectangle->GetBottom(), rectangle->X + r, rectangle->GetBottom());
			pPath->AddArc(rectangle->X, rectangle->GetBottom() - l, (float)l, (float)l, 90.0f, 90.0f);
            pPath->AddLine(rectangle->X, rectangle->GetBottom() - r, rectangle->X, rectangle->Y + r);
            pPath->AddArc(rectangle->X, rectangle->Y, (float)l, (float)l, 180.0f, 90.0f);

			return;
		}
GpStatus CreateBitmap(GpBitmap **Bitmap, long Width, long Height, PixelFormat pixelformat)
{
    Gdiplus::DllExports::GdipCreateBitmapFromScan0(Width, Height, 0, pixelformat, NULL, Bitmap);
	return Ok;
}

GpStatus CreateBitmapWithGraphics(GpBitmap **Bitmap, GpGraphics **graphics, long Width, long Height, PixelFormat pixelformat)
{
	Gdiplus::DllExports::GdipCreateBitmapFromScan0(Width, Height, 0, pixelformat, NULL, Bitmap);
    Gdiplus::DllExports::GdipGetImageGraphicsContext(*Bitmap, graphics);
	return Ok;
}


void CreateGlowTextBitmap(Bitmap **TextBmp, Gdiplus::Font *font, FontFamily *fontfamily, FontStyle fontstyle, 
						  float FontSize, StringFormat *strformat,const wchar_t *Text,int cchText,long BmpWidth,long BmpHeight, 
						  Color GlowColor, float GlowRange, float GlowSize, bool DrawInner, 
						  Brush *InnerBrush,float offsetX, float offsetY, bool DrawRange)
{
    if (GlowRange == 0.0)
		return;

    /*GpGraphics *GlowGraphics,*TextGraphics;
	GpMatrix *GlowMatrix,*TextMatrix;
	GpPen *GlowPen;
	GpSolidFill *GlowBrush;
	GpPath *TextPath;
    GpBitmap *GlowBmp;*/
	

	RectF TextRect(GlowSize + GlowRange,GlowSize + GlowRange,BmpWidth - GlowSize - GlowRange,BmpHeight - GlowSize - GlowRange);
    
	/*
    With TextRect
        .Left = GlowSize + GlowRange
        .Top = .Left
        .Right = BmpWidth - GlowSize - GlowRange
        .Bottom = BmpHeight - GlowSize - GlowRange
    End With*/
    
    //CreateBitmapWithGraphics(&GlowBmp, &GlowGraphics, (long)((REAL)(BmpWidth) / GlowRange), (long)((REAL)(BmpHeight) / GlowRange));
    Bitmap GlowBmp((long)((REAL)(BmpWidth) / GlowRange),(long)((REAL)(BmpHeight) / GlowRange));
	Graphics GlowGraphics(&GlowBmp);
	//Gdiplus::DllExports::GdipSetSmoothingMode(GlowGraphics, SmoothingModeHighQuality);
    GlowGraphics.SetSmoothingMode(SmoothingModeHighQuality);
	if (DrawRange)
	{
    //Gdiplus::DllExports::GdipCreateMatrix(&GlowMatrix);
	Matrix GlowMatrix;
    //Gdiplus::DllExports::GdipSetMatrixElements(GlowMatrix, 1 / GlowRange, 0, 0, 1 / GlowRange, -1 / GlowRange, -1 / GlowRange);
	GlowMatrix.SetElements(1 / GlowRange, 0, 0, 1 / GlowRange, -1 / GlowRange, -1 / GlowRange);
    //Gdiplus::DllExports::GdipSetWorldTransform(GlowGraphics, GlowMatrix);
	GlowGraphics.SetTransform(&GlowMatrix);
    //Gdiplus::DllExports::GdipDeleteMatrix(GlowMatrix);
    
    //Gdiplus::DllExports::GdipCreatePen1(GlowColor.GetValue(), GlowSize, UnitPixel, &GlowPen);
	Pen GlowPen(GlowColor,GlowSize);
    //Gdiplus::DllExports::GdipCreateSolidFill(GlowColor.GetValue(), &GlowBrush);
	SolidBrush GlowBrush(GlowColor);
    //Gdiplus::DllExports::GdipCreatePath(FillModeAlternate, &TextPath);
	GraphicsPath TextPath(FillModeAlternate);
    
    //Gdiplus::DllExports::GdipAddPathString(TextPath, Text->GetString(), Text->GetLength(), fontfamily, fontstyle, FontSize, &TextRect, strformat);
	TextPath.AddString(Text, cchText, fontfamily, fontstyle, FontSize, TextRect, strformat);
	//Gdiplus::DllExports::GdipDrawPath(GlowGraphics, GlowPen, TextPath);
	GlowGraphics.DrawPath(&GlowPen, &TextPath);
    //Gdiplus::DllExports::GdipFillPath(GlowGraphics, GlowBrush, TextPath);
	GlowGraphics.FillPath(&GlowBrush, &TextPath);
	}
    /*Gdiplus::DllExports::GdipDeleteBrush(GlowBrush);
    Gdiplus::DllExports::GdipDeletePen(GlowPen);
	Gdiplus::DllExports::GdipDeleteGraphics(GlowGraphics);*/

	
    //CreateBitmapWithGraphics(TextBmp, &TextGraphics, BmpWidth, BmpHeight);
	Bitmap tTextBmp(BmpWidth, BmpHeight);
	Graphics TextGraphics(&tTextBmp);
    //Gdiplus::DllExports::GdipSetSmoothingMode(TextGraphics, SmoothingModeHighQuality);
	TextGraphics.SetSmoothingMode(SmoothingModeHighQuality);
    //Gdiplus::DllExports::GdipSetTextRenderingHint(TextGraphics, TextRenderingHintAntiAliasGridFit);
	TextGraphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
    //Gdiplus::DllExports::GdipSetInterpolationMode(TextGraphics, InterpolationModeHighQualityBilinear);
    TextGraphics.SetInterpolationMode(InterpolationModeHighQualityBilinear);
	if (DrawRange)
	{
    //Gdiplus::DllExports::GdipCreateMatrix(&TextMatrix);
	Matrix TextMatrix;
    //Gdiplus::DllExports::GdipSetMatrixElements(TextMatrix, 1, 0, 0, 1, 0, 0);
    TextMatrix.SetElements(1, 0, 0, 1, 0, 0);
	//Gdiplus::DllExports::GdipDrawImageRect(TextGraphics, GlowBmp, offsetX + 1, offsetY, (REAL)(BmpWidth - 2), (REAL)BmpHeight);
    TextGraphics.DrawImage(&GlowBmp, offsetX + 1, offsetY, (REAL)(BmpWidth - 2), (REAL)BmpHeight);
	}

    //'NOTICE: 为了增加小字清晰度，主文字绘制采用DrawString。
    //'因此对发光还有修正（参照小字体修正），于是在大号下发光会有偏差！
    //'如果对于清晰度要求不是很高，请将下句修改为Path的绘制
    if (DrawInner)
	{
		//Gdiplus::DllExports::GdipDrawString(TextGraphics, Text->GetString(), Text->GetLength(), font, &TextRect, strformat, InnerBrush);
		TextGraphics.DrawString(Text, cchText, font, TextRect, strformat, InnerBrush);
	}

    /*Gdiplus::DllExports::GdipDeleteMatrix(TextMatrix);
    Gdiplus::DllExports::GdipDeletePath(TextPath);
    Gdiplus::DllExports::GdipDisposeImage(GlowBmp);*/

	//这句话可不用
	//Gdiplus::DllExports::GdipDeleteGraphics(TextGraphics);
	*TextBmp = tTextBmp.Clone(0,0,BmpWidth, BmpHeight,PixelFormat32bppARGB);

}

		BYTE MixAlpha(BYTE fade, BYTE alpha)
		{
			BYTE fade_ = fade;
			if (alpha == 0)
			{
				return 0;
			} else if (alpha != 255) {
				//非饱和
				fade_ = (BYTE)(fade_ * alpha / 255);
			}
			return fade_;
		}

	};
};