
#include "common.h"
#include "global.h"

#include <base/file/file.h>

//#include "resource.h"
#include "frame/frame_theme.h"
#include "frame/messageloop.h"
#include "frame/skin_data.h"

#include "view_gdiplus.h"
#include "view_skia.h"

// 全局变量:
namespace global{
	namespace Gdip {
		wchar_t *szFontName[] = {L"微软雅黑", L"宋体", L"Verdana"};
		::Gdiplus::Font *font = NULL;
		::Gdiplus::Font *font_titletab = NULL;
		FontFamily *fontfamily = NULL;

		SolidBrush *whitebrush = NULL;
		SolidBrush *blackbrush = NULL;

		StringFormat *strfmt = NULL;
		StringFormat *strfmtLC = NULL;

		Pen *buttonlinepen = NULL;
	};

	namespace skia {
		char *szFontName[] = {"Microsoft YaHei", "SimSun", "Verdana"};
		SkTypeface *font = NULL;
	};

	HINSTANCE hInstance = NULL;							// 当前实例
	HWND hMainWindow = NULL;
	HFONT hFont = NULL;

	std::wstring wpath;
	std::string apath;
	
	view::frame::CMainWindow* pMainWindow = NULL;

	void dbg_dump(struct _EXCEPTION_POINTERS* ExceptionInfo)
	{
		char buff[1024];
		memset(buff, 0, sizeof(buff));

		wsprintfA
			(buff, 
			"线程[%04d]出现致命错误。\n错误描述：\nCrash Code:0x%.8x\tAddr=0x%.8x\nFlags=0x%.8x\tParams=0x%.8x\neax=%.8x\tebx=%.8x\tecx=%.8x\nedx=%.8x\tesi=%.8x\tedi=%.8x\neip=%.8x\tesp=%.8x\tebp=%.8x\n",
				::GetCurrentThreadId(),
				ExceptionInfo->ExceptionRecord->ExceptionCode,
				ExceptionInfo->ExceptionRecord->ExceptionAddress,
				ExceptionInfo->ExceptionRecord->ExceptionFlags,
				ExceptionInfo->ExceptionRecord->NumberParameters,
				ExceptionInfo->ContextRecord->Eax,
				ExceptionInfo->ContextRecord->Ebx,
				ExceptionInfo->ContextRecord->Ecx,
				ExceptionInfo->ContextRecord->Edx,
				ExceptionInfo->ContextRecord->Esi,
				ExceptionInfo->ContextRecord->Edi,
				ExceptionInfo->ContextRecord->Eip,
				ExceptionInfo->ContextRecord->Esp,
				ExceptionInfo->ContextRecord->Ebp
			);

		std::wstring debugfile = wpath;
		debugfile += L"debug.log";

		base::CFile file;
		if (file.Open(base::kFileCreate, debugfile.c_str())) {
			file.Write((unsigned char *)buff, strlen(buff));
			file.Close();
		}

		MessageBoxW(NULL, L"哎呀，出现错误了！", L"错误", MB_OK | MB_ICONERROR);

	#ifdef TA_OPEN_DEBUG
		char *info = debug::thread::GetThreadInfo();
		if (info)
		{
			MessageBoxA(NULL, info, "", MB_OK | MB_ICONERROR);
			wsprintfA(buff, "详细描述: %s", info);
			cPrintA(buff, CONSOLE_TEXT_COLOR_RED);
		}

		debug::thread::RemoveThreadInfo();
	#endif
	}

	LONG WINAPI bad_exception(struct _EXCEPTION_POINTERS* ExceptionInfo)
	{
		dbg_dump(ExceptionInfo);
		::TerminateThread(GetCurrentThread(),0);
		return TRUE;
	}

	void Init(HINSTANCE hInst, LPCSTR skin_json_stream)
	{
		InitInstance(hInst);
		InitPath();
		//view font
		view::Gdip::Init();
		view::skia::Init();
		InitFont();

		SetUnhandledExceptionFilter(bad_exception);	//崩溃消息回调

		//初始化主题
		view::frame::theme::InitTheme();
		if (skin_json_stream)
		{
			view::frame::skin::CSkinData::LoadSkinFromStream(skin_json_stream);
		} else {
			view::frame::skin::CSkinData::LoadSkin(L"default");
		}
		view::frame::messageloop::InitLoopThread(1);
	}

	void Uninit()
	{
		global::UninitFont();
		view::frame::skin::CSkinData::UnloadSkin();
		view::Gdip::Uninit();
		view::skia::Uninit();
	}

	void InitInstance(HINSTANCE hInst)
	{
		global::hInstance = hInst; // 将实例句柄存储在全局变量中

		// 如果一个运行在 Windows XP 上的应用程序清单指定要
		// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
		//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
		INITCOMMONCONTROLSEX InitCtrls;
		InitCtrls.dwSize = sizeof(InitCtrls);
		// 将它设置为包括所有要在应用程序中使用的
		// 公共控件类。
		InitCtrls.dwICC = ICC_WIN95_CLASSES;
		InitCommonControlsEx(&InitCtrls);

	}

	void InitPath()
	{
		wchar_t szwpath[MAX_PATH];
		char szapath[MAX_PATH];

		GetModuleFileNameW(NULL, szwpath, MAX_PATH);
		for (int i = lstrlen(szwpath) - 1; i > 0; i--)
		{
			if (szwpath[i] == '\\')
			{
				szwpath[i + 1] = 0;
				break;
			}
		}

		GetModuleFileNameA(NULL, szapath, MAX_PATH);
		for (int i = lstrlenA(szapath) - 1; i > 0; i--)
		{
			if (szapath[i] == '\\')
			{
				szapath[i + 1] = 0;
				break;
			}
		}

		wpath = szwpath;
		apath = szapath;
	}
	
	void InitFont()
	{
		//gdip
		int iFontIndex = 0, iFontCount = sizeof(Gdip::szFontName) / sizeof(wchar_t *);
		Gdip::fontfamily = new FontFamily(Gdip::szFontName[iFontIndex]);
		//Gdiplus::DllExports::GdipCreateFontFamilyFromName(g_FontName, 0, &g_fontfamily);
		while (!Gdip::fontfamily->IsAvailable())
		{
			iFontIndex++;
			if (iFontIndex < iFontCount)
			{
				delete Gdip::fontfamily;
				Gdip::fontfamily = new FontFamily(Gdip::szFontName[iFontIndex]);
			}
			else
			{
				iFontIndex = 0;
				//MsgBoxError(_T("没有可得到的字体！"));
				MessageBoxW(NULL, L"没有可用的字体！", L"错误", MB_ICONERROR | MB_OK);
				break;
			}
		}

		Gdip::font = new Gdiplus::Font(Gdip::fontfamily, 13, 0, UnitPixel);
		Gdip::font_titletab = new Gdiplus::Font(Gdip::fontfamily, 18, 0, UnitPixel);
		//Gdiplus::FontStyleBold

		HFONT hDefaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT); // Gets the default font object // 取得默认字体
		LOGFONT font;
		
		memset(&font, 0, sizeof(LOGFONT));

		GetObject(hDefaultFont, sizeof(LOGFONT), &font);
		DeleteObject(hDefaultFont);

		lstrcpy(font.lfFaceName, Gdip::szFontName[iFontIndex]);	//L"微软雅黑"

		hFont = CreateFontIndirect(&font);

		char szfontfile[MAX_PATH];
		lstrcpyA(szfontfile, global::apath.c_str());
		lstrcatA(szfontfile, "skin/default.ttf");
		skia::font = SkTypeface::CreateFromFile(szfontfile);//SkTypeface::CreateFromName(skia::szFontName[iFontIndex], SkTypeface::kNormal);
		if (!skia::font) {
			skia::font = SkTypeface::CreateFromName(skia::szFontName[iFontIndex], SkTypeface::kNormal);
		}

		Gdip::whitebrush = new SolidBrush(0xffffffff);
		Gdip::blackbrush = new SolidBrush(0xff000000);

		Gdip::strfmt = new StringFormat();
		Gdip::strfmt->SetAlignment(StringAlignmentCenter);
		Gdip::strfmt->SetLineAlignment(StringAlignmentCenter);

		Gdip::strfmtLC = new StringFormat();
		Gdip::strfmtLC->SetAlignment(StringAlignmentNear);
		Gdip::strfmtLC->SetLineAlignment(StringAlignmentCenter);

		Gdip::buttonlinepen = new Pen(Color::MakeARGB(255,75,189,237));
	}

	void UninitFont()
	{
		if (Gdip::fontfamily) delete Gdip::fontfamily;
		if (Gdip::font) delete Gdip::font;
		if (Gdip::whitebrush) delete Gdip::whitebrush;
		if (Gdip::blackbrush) delete Gdip::blackbrush;

		if (Gdip::strfmt) delete Gdip::strfmt;
		if (Gdip::buttonlinepen) delete Gdip::buttonlinepen;

		if (skia::font) skia::font->unref();
	}
};