
#include "base/global.h"
#include "base/view.h"
#include "base/common.h"
#include "resource.h"

#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_child.h"
#include "base/frame/frame_theme.h"
#include "base/frame/skin_data.h"
#include "imagelist.h"

namespace view{
	namespace frame{
		CImageList::CImageList()
			: m_hImageList(NULL)
		{
		}
		CImageList::~CImageList()
		{
			Destroy();
		}

		bool CImageList::Create(int cx, int cy, UINT nFlags, int nInitial, int nGrow)
		{
			m_hImageList = ImageList_Create(cx, cy, nFlags, nInitial, nGrow);
			return (m_hImageList != NULL);
		}

		int CImageList::LoadIconFromResource(UINT id)
		{
			HICON hIcon = LoadIcon(global::hInstance, MAKEINTRESOURCE(id));
			int ret = AddIcon(hIcon);
			DeleteObject(hIcon);
			return ret;
		}

		int CImageList::GdipLoadFromResource(LPCWSTR type, UINT id)
		{
			Image *img = NULL;
			if(GdipLoadImageFromResource(&img, type, id))
			{
				int width = img->GetWidth();
				int height = img->GetHeight();

				int cx, cy, count = 0;
				if (GetIconSize(&cx, &cy))
				{
					if (width >= cx && height >= cy)
					{
						//·Ö¸îÍ¼Æ¬
						for (int y = 0; y < height; y += cy)
						{
							for (int x = 0; x < width; x += cx)
							{
								Bitmap bmIcon(cx, cy);
								Graphics imgraphics(&bmIcon);
								HICON hIcon = NULL;
								imgraphics.DrawImage(img, 0, 0, x, y, cx, cy, UnitPixel);
								bmIcon.GetHICON(&hIcon);

								AddIcon(hIcon);
								
								::DeleteObject(hIcon);
								count++;
							}
						}
					}
				}
				delete img;
				return count;
			} else {
				return -1;
			}
		}

		int CImageList::GetImageCount() const
		{
			return ImageList_GetImageCount(m_hImageList);
		}

		int CImageList::Add(HBITMAP hbmImage, HBITMAP hbmMask)
		{
			return ImageList_Add(m_hImageList, hbmImage, hbmMask);
		}

		int CImageList::AddIcon(HICON hicon)
		{
			return ImageList_AddIcon(m_hImageList, hicon);
		}

		int CImageList::ReplaceIcon(int i, HICON hicon)
		{
			return ImageList_ReplaceIcon(m_hImageList, i, hicon);
		}

		bool CImageList::Remove(int i)
		{
			return ImageList_Remove(m_hImageList, i);
		}

		bool CImageList::RemoveAll()
		{
			return ImageList_RemoveAll(m_hImageList);
		}

		bool CImageList::GetIconSize(int *cx, int *cy)
		{
			return ImageList_GetIconSize(m_hImageList, cx, cy);
		}

		bool CImageList::SetIconSize(int cx, int cy)
		{
			return ImageList_SetIconSize(m_hImageList, cx, cy);
		}

		bool CImageList::Draw(int i, HDC hdcDst, int x, int y, UINT fStyle)
		{
			return ImageList_Draw(m_hImageList, i, hdcDst, x, y, fStyle);
		}

		bool CImageList::DrawEx(int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
		{
			return ImageList_DrawEx(m_hImageList, i, hdcDst, x, y, dx, dy, rgbBk, rgbFg, fStyle);
		}

		HICON CImageList::GetIcon(int i, UINT flags)
		{
			return ImageList_GetIcon(m_hImageList, i, flags);
		}

		HICON CImageList::ExtractIcon(int i)
		{
			return ImageList_ExtractIcon(global::hInstance, m_hImageList, i);
		}

		bool CImageList::Destroy()
		{
			if (m_hImageList)
			{
				ImageList_Destroy(m_hImageList);
				m_hImageList = NULL;
			}
			return true;
		}

	};
};