
#ifndef _SNOW_CUTE_SKIN_DATA_H_
#define _SNOW_CUTE_SKIN_DATA_H_ 1

#include <vector>

#include "frame_window.h"
#include "frame_skia.h"

#include "../common.h"
#include "captionbutton.h"

class DictionaryValue;
class Value;

class SkBitmap;

#define LINK_STATE_COUNT 3

namespace view {
	namespace frame {

		extern bool LoadStream(IStream** pIStream, void** pResBuffer, LPCWSTR type, const UINT id, DWORD *size = NULL);
		extern bool GdipLoadImageFromResource(Image** img, LPCWSTR type, UINT id);

		extern bool SkiaLoadImageFromStream(SkBitmap** img, SkStream *stream);
		extern bool SkiaLoadImageFromResource(SkBitmap** img, LPCWSTR type, UINT id);
		extern bool SkiaLoadImageFromFile(SkBitmap** img, LPCWSTR path);
		
		namespace skin {
			enum CAPTION_BUTTON_STATE{
				kCBNormal = 0,
				kCBHover,
				kCBDown,
			};

			typedef struct _LINEAR_COLOR{
				int n;
				std::vector<float> pos;
				std::vector<Color> colors;
				int linear_mode;
			} LINEAR_COLOR;

			typedef struct _CAPTION_BUTTON_DRAW_INFO{
				bool enable;
				LINEAR_COLOR lcolor;
				//usefile
				int x;
				int y;
			} CAPTION_BUTTON_DRAW_INFO;

			typedef struct _CAPTION_BUTTON_INFO{
				int width;
				int height;
				//"normal", "hover", "down"
				CAPTION_BUTTON_DRAW_INFO draw[3];
			} CAPTION_BUTTON_INFO;

			typedef struct _BACKGROUND_INFO{
				bool usefile;
				std::wstring path;
				Image *img;
				SkBitmap *skimg;
				LINEAR_COLOR lcolor;
			} BACKGROUND_INFO;

			typedef struct _CAPTION_INFO{
				int height;
				Color text_color;
				SolidBrush *text_brush;
				int button_spacing;
				bool button_draw_usefile;
				Image *button_img;
				SkBitmap *button_skimg;
				CAPTION_BUTTON_INFO cbi[3];
			} CAPTION_INFO;

			typedef struct _STAITC_INFO{
				Color main_color;
				SolidBrush *main_brush;
				Color view_color;
				SolidBrush *view_brush;
				Color dlg_color;
				SolidBrush *dlg_brush;
			} STAITC_INFO;

			typedef struct _PBAR_INFO{
				Color sidecolor;
				SolidBrush *sidebrush;
				LINEAR_COLOR background;
				LINEAR_COLOR show;
			} PBAR_INFO;

			typedef struct _DIALOG_INFO{
				LINEAR_COLOR background;
			} DIALOG_INFO;

			typedef struct _TITLETAB_INFO{
				std::vector<Color> colors;
				Color select_sidecolor;
				Color select_text_color;
				SolidBrush *select_text_brush;
				Color background_text_color;
				LINEAR_COLOR select_lcolor;
				int select_alpha;
			} TITLETAB_INFO;

			typedef struct _LINK_STATE_INFO{
				Color color;
				bool underline;
			} LINK_STATE_INFO;

			typedef struct _LINK_INFO{
				bool bload;
				LINK_STATE_INFO si[LINK_STATE_COUNT];
			} LINK_INFO;

			typedef struct _BUTTON_STATE_INFO{
				Color sidecolor;
				LINEAR_COLOR lc;
			} BUTTON_STATE_INFO;

			typedef struct _BUTTON_INFO{
				Color text_color;
				SolidBrush *text_brush;
				// normal hover down unenable
				BUTTON_STATE_INFO bsi[4];
			} BUTTON_INFO;

			typedef struct _SkinBase {
				std::wstring dirname;
				std::wstring name;
				std::wstring author;
			} SkinBase;

			typedef std::vector<SkinBase> SkinBaseList;

			extern int ListSkin(SkinBaseList& list, LPCWSTR skinpath);

			class CSkinData{
			public:
				CSkinData();
				virtual ~CSkinData();

				//def: SK_ColorBLACK
				static SkColor GetStaticTextColor(HWND hWnd);

				static void OnDrawBackground(Graphics *g, CWindow *pWindow, int width, int height);

				static void OnSetCaptionButton(CWindow *pWindow, CCaptionButton cbutton[3]);
				static void OnDrawCaptionButton(Graphics *g, CCaptionButton *pbutton);
				static void OnDrawCaptionButton(Graphics *g, CWindow *pWindow, CCaptionButton cbutton[3], int caption_width, int caption_height);
				static void OnDrawCaptionText(Graphics *g, CWindow *pWindow, int caption_width, int caption_height, int text_size = 13);

				static void OnDrawDialogBackground(SkCanvas& canvas, CWindow *pWindow, int width, int height);
				static void OnDrawBackground(SkCanvas& canvas, CWindow *pWindow, int width, int height);
				static void OnDrawCaptionButton(SkCanvas& canvas, CCaptionButton *pbutton, SkRect *playout = NULL);
				static void OnDrawCaptionText(SkCanvas& canvas, CWindow *pWindow, int caption_width, int caption_height, int text_size = 13);
				
				static void OnSetRegion(CWindow *pWindow, int width, int height);

				static void ClearLinearColor(LINEAR_COLOR *lc);
				static bool LoadLinearColor(DictionaryValue* linearValue, LINEAR_COLOR *lc);
				static bool LoadButtonInfo(DictionaryValue* v);
				static bool LoadCaptionInfo(DictionaryValue* v);
				static bool LoadCBInfo(DictionaryValue* cbValue, CAPTION_BUTTON_INFO *cbInfo);
				static bool LoadPBarInfo(DictionaryValue* v);
				static bool LoadTitleTabInfo(DictionaryValue* v);
				static bool LoadLinkInfo(DictionaryValue* v);
				static bool LoadDialogInfo(DictionaryValue* v);
				static bool LoadViewInfo(DictionaryValue* v);
				static bool LoadMainInfo(DictionaryValue* v);
				static bool LoadSkin(LPCWSTR lpszDirName);
				static bool LoadSkinFromStream(LPCSTR json_stream);
				static bool LoadSkinFromValue(::Value *v);
				static void UnloadSkin();

				static Color StringToColor(std::string& str);
			//protected:

				static bool m_enable;
				static std::wstring m_name;
				static CAPTION_INFO m_ci;
				static STAITC_INFO m_si;
				static PBAR_INFO m_pi;
				static BACKGROUND_INFO m_bi;
				static TITLETAB_INFO m_tti;
				static LINK_INFO m_li;
				static BUTTON_INFO m_btni;
				static DIALOG_INFO m_di;
				static std::wstring m_skin_path;
			};
		};
	};
};

#endif