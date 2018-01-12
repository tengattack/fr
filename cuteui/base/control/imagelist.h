

#ifndef _SNOW_CUTE_IMAGELIST_H_
#define _SNOW_CUTE_IMAGELIST_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"

namespace view{
	namespace frame{
		
		class CImageList {
		public:
			CImageList();
			~CImageList();

			bool Create(int cx, int cy, UINT nFlags, int nInitial, int nGrow);

			int GdipLoadFromResource(LPCWSTR type, const UINT id);
			int LoadIconFromResource(UINT id);

			int Add(HBITMAP hbmImage, HBITMAP hbmMask);
			int AddIcon(HICON hicon);

			int ReplaceIcon(int i, HICON hicon);

			bool Remove(int i);
			bool RemoveAll();
			bool Destroy();

			bool GetIconSize(int *cx, int *cy);
			bool SetIconSize(int cx, int cy);

			bool Draw(int i, HDC hdcDst, int x, int y, UINT fStyle);
			bool DrawEx(int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);

			HICON GetIcon(int i, UINT flags);
			HICON ExtractIcon(int i);

			int GetImageCount() const;

			inline HIMAGELIST GetImageList()
			{
				return m_hImageList;
			}
		
		protected:
			HIMAGELIST m_hImageList;
		};
	};
};

#endif