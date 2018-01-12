
#ifndef _SNOW_CUTE_GLOBAL_H_
#define _SNOW_CUTE_GLOBAL_H_ 1

#include "common.h"
#include "frame/frame_window.h"
#include "frame/mainwindow.h"

#include <string>

// 全局变量:
namespace global{

	extern HINSTANCE hInstance;					// 当前实例
	extern HFONT hFont;

	extern HWND hMainWindow;

	extern view::frame::CMainWindow* pMainWindow;

	extern std::wstring wpath;
	extern std::string apath;

	void Init(HINSTANCE hInst, LPCSTR skin_json_stream = NULL);
	void Uninit();

	void InitInstance(HINSTANCE hInst);

	void InitPath();

	//font
	void InitFont();
	void UninitFont();

	enum SnowFadeEventID
	{
		kTimerFadeIn = 1000,
		kTimerFadeOut,
		kTimerFadeDown,
	};

	namespace Gdip {
		extern ::Gdiplus::Font *font;
		extern ::Gdiplus::Font *font_titletab;
		extern FontFamily *fontfamily;

		extern SolidBrush *whitebrush;
		extern SolidBrush *blackbrush;

		extern StringFormat *strfmt;
		extern StringFormat *strfmtLC;

		extern Pen *buttonlinepen;
	};

	namespace skia {
		extern SkTypeface *font;
	};
};

#endif