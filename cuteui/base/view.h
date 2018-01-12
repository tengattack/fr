
#ifndef _SNOW_CUTE_VIEW_H_
#define _SNOW_CUTE_VIEW_H_ 1

//获取内存设备句柄（画背景）
#define WM_TA_GET_SKBITMAP			(WM_USER + 0x01)
#define WM_TA_REDRAW_BACKGROUND		(WM_USER + 0x02)
#define WM_TA_TRAY_NOTIFY			(WM_USER + 0x03)

#define WM_TA_USER					(WM_USER + 0x04)

namespace view{

	int MessageLoop();

	ATOM RegisterClass(LPCWSTR lpszClassName, HICON hBigIcon, HICON hSmallIcon);

	LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

};

#endif