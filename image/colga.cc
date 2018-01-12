
#include "colga.h"
#include <stdio.h>
#include <windows.h>

namespace image {

static int s_last_color = 0;

bool IsColgaEdit(HWND hWnd)
{
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	
	TCHAR buffer[64];
	buffer[0] = 0;
	if (GetClassName(hWnd, buffer, 64)) {
		if (_tcsstr(buffer, _T("EDIT"))) {
			return true;
		}
	}
	/*if (dwStyle == 0x560100c0) {
		return true;
	}*/
	return false;
}

HWND FindColorEdit(HWND hWnd, int *issecond)
{
	HWND hNextWnd = hWnd;
	HWND hChildWnd = NULL;

	while (hNextWnd != NULL) {

		if (IsColgaEdit(hNextWnd)) {
			//第二个才是ColorEdit
			if (*issecond) {
				return hNextWnd;
			} else {
				*issecond = 1;
			}
		}

		hChildWnd = ::GetWindow(hNextWnd, GW_CHILD);
		if (hChildWnd) {
			HWND hFindWnd = FindColorEdit(hChildWnd, issecond);
			if (hFindWnd) {
				return hFindWnd;
			}
		}

		hNextWnd = ::GetNextWindow(hNextWnd, GW_HWNDNEXT);
	}

	return NULL;
}

bool colga_set_color(LPCSTR lpszFile, uint32 new_colors)
{
	if (new_colors == 0) {
		return false;
	}
	HWND hCOLGAWnd = FindWindow(NULL, _T("COLGA"));
	if (hCOLGAWnd == NULL) {
		return false;
	}

	SetForegroundWindow(hCOLGAWnd);
	keybd_event(VK_CONTROL,0,0,0);       
	keybd_event('O',0,0,0);
	Sleep(200);
	keybd_event('O',0,KEYEVENTF_KEYUP,0);
	keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);       
	
	Sleep(1500);

	if (OpenClipboard(NULL)) {  
		HANDLE hClip;
		char * pBuf;
		EmptyClipboard();

		hClip = GlobalAlloc(GMEM_MOVEABLE, lstrlenA(lpszFile) + 1);
		pBuf = (char *)GlobalLock(hClip);
		lstrcpyA(pBuf, lpszFile);
		GlobalUnlock(hClip);
		SetClipboardData(CF_TEXT, hClip);
		CloseClipboard();
    }

	//SetForegroundWindow((HWND)0x00030532);
	keybd_event(VK_CONTROL,0,0,0);       
	keybd_event('V',0,0,0);
	Sleep(200);
	keybd_event('V',0,KEYEVENTF_KEYUP,0);
	keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);       
	

	keybd_event(VK_RETURN,0,0,0);
	Sleep(30);
	keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);   

	Sleep(600);

	//SetForegroundWindow(hCOLGAWnd);
	//减色
	keybd_event(VK_CONTROL,0,0,0);       
	keybd_event('R',0,0,0);
	Sleep(200);
	keybd_event('R',0,KEYEVENTF_KEYUP,0);
	keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);       
	

	Sleep(600);

	HWND hWnd = FindWindow(NULL, _T("減色オプション"));
	if (hWnd == NULL) {
		Sleep(600);
		hWnd = FindWindow(NULL, _T("減色オプション"));
		if (hWnd == NULL) return false;
	}
	HWND hOkWnd = FindWindowEx(hWnd, NULL, NULL, _T("OK"));
	if (hOkWnd == NULL) {
		return false;
	}
	SetForegroundWindow(hWnd);

	if (s_last_color != new_colors) {
		s_last_color = new_colors;

		int secondflag = 0;
		HWND hColorEdit = FindColorEdit(hWnd, &secondflag);
		if (!hColorEdit) {
			return false;
		}

		char szColors[10];
		wsprintfA(szColors, "%d", new_colors);
		if (SendMessageA(hColorEdit, WM_SETTEXT, NULL, (LPARAM)szColors)) {
			printf("SetColors -> (%s)\n", szColors);
		} else {
			return false;
		}

#if 0
		HWND hChildWnd = ::GetWindow(hWnd, GW_CHILD);
		bool bfind = false;

		while(hChildWnd != NULL) {
			HWND hEditWnd = ::FindWindowEx(hChildWnd, NULL, NULL, _T("256"));
			if (hEditWnd == NULL) {
				hEditWnd = ::FindWindowEx(hChildWnd, NULL, NULL, _T("16"));
			}
			if (hEditWnd != NULL)
			{
				//::SetForegroundWindow(hEditWnd);
				//::SetFocus(hEditWnd);
				::SendMessage(hEditWnd, WM_SETFOCUS, NULL, NULL);
				Sleep(500);

				keybd_event(VK_CONTROL,0,0,0);       
				keybd_event('A',0,0,0);
				Sleep(200);
				keybd_event('A',0,KEYEVENTF_KEYUP,0);
				keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);       
				

				//不知为何会失败，.net的缘故么？
				SetWindowTextA(hEditWnd, szColors);

				if (OpenClipboard(NULL)) {  
					HANDLE hClip;
					char * pBuf;
					EmptyClipboard();

					hClip = GlobalAlloc(GMEM_MOVEABLE, lstrlenA(szColors) + 1);
					pBuf = (char *)GlobalLock(hClip);
					lstrcpyA(pBuf, szColors);
					GlobalUnlock(hClip);
					SetClipboardData(CF_TEXT, hClip);
					CloseClipboard();
				}

				//SetWindowText(hChildWnd, szColors);

				keybd_event(VK_CONTROL,0,0,0);       
				keybd_event('V',0,0,0);
				Sleep(200);
				keybd_event('V',0,KEYEVENTF_KEYUP,0);
				keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);       
				
				//std::cout << "SetColors -> (" << szColors << ")" << std::endl;
				
				//Sleep(5000);
				bfind = true;
				break;
			}
			//继续访问下一个子窗体
			hChildWnd = ::GetNextWindow(hChildWnd, GW_HWNDNEXT);
		}
		if (!bfind) {
			return false;
		}
#endif
	}

	SetForegroundWindow(hWnd);

	//按下Ok按钮
	SendMessage(hOkWnd, WM_LBUTTONDOWN, MK_LBUTTON, NULL);
	Sleep(100);
	SendMessage(hOkWnd, WM_LBUTTONUP, MK_LBUTTON, NULL);

	Sleep(600);

	keybd_event(VK_RETURN,0,0,0);
	Sleep(30);
	keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);      

	//保存
	keybd_event(VK_CONTROL,0,0,0);       
	keybd_event('S',0,0,0);
	Sleep(200);
	keybd_event('S',0,KEYEVENTF_KEYUP,0);
	keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);       
	

	Sleep(600);

	SetForegroundWindow(hCOLGAWnd);
	//关闭
	keybd_event(VK_CONTROL,0,0,0);       
	keybd_event('W',0,0,0);
	Sleep(200);
	keybd_event('W',0,KEYEVENTF_KEYUP,0);
	keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);       
	

	return true;
}

}