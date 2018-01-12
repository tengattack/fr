
#include "global.h"
#include "common.h"
#include "resource.h"
#include "view.h"

#include "frame/frame.h"
#include "frame/frame_window.h"
#include "frame/messageloop.h"

namespace view{

	int MessageLoop()
	{
		return frame::messageloop::Loop();
	}

//
//  函数: RegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM RegisterClass(LPCWSTR lpszClassName, HICON hBigIcon, HICON hSmallIcon)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= global::hInstance;
	wcex.hIcon			= hBigIcon;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);	//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= lpszClassName;
	wcex.hIconSm		= hSmallIcon;

	return RegisterClassEx(&wcex);
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	frame::CWindow *pWindow = frame::GetWindow(hWnd);
	
	if (pWindow)
	{
		//检查是否在内处理
		bool handled = false;
		LRESULT result = pWindow->OnWndProc(message, wParam, lParam, handled);
		if (handled)
			return result;
	}

	switch (message)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps = {0};
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	/*case WM_NCDESTROY:
		//非客户区销毁
		DefWindowProc(hWnd, message, wParam, lParam);

		break;*/
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

};