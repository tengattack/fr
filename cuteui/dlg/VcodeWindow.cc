
#include "stdafx.h"
#include "base/common.h"
#include "base/view.h"
#include "base/frame/frame.h"
#include "base/frame/frame_window.h"
#include "base/frame/frame_skia.h"
#include "base/frame/dialog.h"

//control
#include "base/control/button.h"
#include "base/control/radio.h"
#include "base/control/check.h"
#include "base/control/text.h"
#include "base/control/listview.h"

#include "base/operation/fileselect.h"

#include "base/string/stringprintf.h"
#include "base/string/string_number_conversions.h"

#include "base/lock.h"

#include "ocr/ocr_base.h"

#include <common/strconv.h>

#include "vcode/vcode-queue.h"
#include "VcodeWindow.h"

#define TEXT_OCRING L"..."

static Lock m_callback_lock;
static CVerifyCodeQueue m_queue(_TA_VCODE_COUNT);	//5个验证码

#ifdef _TA_VCODE_USE_SKIA
	#define COPY_BACKGROUND() \
			SkCanvas canvas(m_vcode_list); \
			SkRect srcrect = {_TA_VCODE_LEFT, _TA_VCODE_TOP, _TA_VCODE_LEFT + _TA_VCODE_WIDTH, _TA_VCODE_TOP + _TA_VCODE_ALL_HEIGHT}; \
			SkRect dstrect = {0, 0, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT}; \
			canvas.drawBitmapRectToRect(m_skbitmap, &srcrect, dstrect, NULL);		
#else
	#define COPY_BACKGROUND() doSkPaint(m_hVcodeMemDC, 0, 0, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT, _TA_VCODE_LEFT, _TA_VCODE_TOP);
			/*m_MemDC.FillSolidRect(0, 0, 
					_TA_VCODE_WIDTH,_TA_VCODE_ALL_HEIGHT, TA_BK_RGB);*/
			doSkPaint(m_hVcodeMemDC, 0, 0, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT, _TA_VCODE_LEFT, _TA_VCODE_TOP);
			//::BitBlt(m_hVcodeMemDC, 0, 0, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT, m_hMemDC, _TA_VCODE_LEFT, _TA_VCODE_TOP, SRCCOPY);
			//::BitBlt(hdc, _TA_VCODE_LEFT, _TA_VCODE_TOP, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT, m_hVcodeMemDC, 0, 0, SRCCOPY);
#endif

#if !defined(SNOW) && !defined(VCODE_WINDOW_OVERWRITE)
extern view::dlg::CVcodeWindow *g_pVCodeDlg;

int WINAPI VcodeChangeCallback(BOOL bSetInfo)
{
	if (g_pVCodeDlg)
	{
		return g_pVCodeDlg->VcodeChangeCallback(bSetInfo);
	} else {
		return 0;
	}
}
#endif

enum _ID_CONTROL{

	ID_TIMER_ANIMATION = 2001,

	//button
	ID_BUTTON_JMP = 4001,
	//Control
	ID_TEXT_VCODE,
	ID_TEXT_INPUTTIMES,

	ID_CHECK_AUTOJMP,

	//Static
	ID_STATIC_INPUTTIMES
};

namespace view {
	namespace dlg {

		const int CVcodeWindow::kGifVcodeElapse = 50;

		CVcodeWindow::CVcodeWindow()
			: m_enable_close(true)
			, m_hide(false)
			, m_first_paint(true)
      , m_first_ocr_show(false)
			, m_inited(false)
			, m_jmpall(false)
#ifndef _TA_VCODE_USE_SKIA
			, m_hVcodeMemDC(NULL)
			, m_hVcodeMemBitmap(NULL)
#endif
			, m_dwInputTimes(0)
			, m_edit_oldWndProc(NULL)
			, m_char_select(NULL)
		{
			LOGFONT m_lf;

			GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT),sizeof(LOGFONT),&m_lf); 
			lstrcpy(m_lf.lfFaceName,_T("微软雅黑"));//字体名；
			m_lf.lfHeight = 35;
			m_lf.lfWeight = 700;	//粗体 重量为700
			m_hVcodeFont = CreateFontIndirect(&m_lf);
		}

		CVcodeWindow::~CVcodeWindow()
		{
#ifndef _TA_VCODE_USE_SKIA
			if (m_hVcodeMemDC) DeleteDC(m_hVcodeMemDC);
#endif
			delete m_char_select;
		}

		void CVcodeWindow::AdjustCharacterSelect(int x, int y)
		{
			m_char_select->Move(x + _TA_VCODE_LEFT + _TA_VCODE_WIDTH, y + _TA_VCODE_TOP);
		}

		void CVcodeWindow::ShowCharacterSelect(CVerifyCode *pvc)
		{
			m_char_select->LoadFriendlyImage(pvc ? pvc->getSkBitmap() : NULL);
			m_char_select->Show(true);
		}

		void CVcodeWindow::CheckCharacterSelect()
		{
			if (m_queue.GetCount() <= 0 || m_queue.GetVerifyCodeKind(0) != _VCODE_FRIENDLY) {
				HideCharacterSelect();
			} else {
				ShowCharacterSelect(m_queue.GetVerifyCode(0));
			}
		}

		void CVcodeWindow::CharacterSelect(int i)
		{
			// 1 to 9
			text[0].ReplaceSel(base::IntToString16(i).c_str());
			//SetFocus();
			text[0].SetFocus();
		}

		void CVcodeWindow::Close() {
			while (m_queue.GetCount() > 0) {
				JmpVcode();
			}
			EnableClose(true);
			m_char_select->Close();
			CWindow::Close();
		}

		void CVcodeWindow::InitWindow()
		{
			AddStaticText(ID_STATIC_INPUTTIMES, 5, _TA_VCODE_TOP + _TA_VCODE_ALL_HEIGHT + 7, 120, 25, L"验证码输入次数:");
			text[1].CreateText(m_hWnd, L"0", ID_TEXT_INPUTTIMES, 125, _TA_VCODE_TOP + _TA_VCODE_ALL_HEIGHT + 5, 60, 25, WS_CHILD | WS_DISABLED | ES_AUTOHSCROLL | ES_CENTER);

			text[0].CreateText(m_hWnd, L"", ID_TEXT_VCODE, 26, 10, 140, 40, WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_CENTER);
			::SendMessage(text[0].hWnd(), WM_SETFONT, (WPARAM)m_hVcodeFont, MAKELPARAM(FALSE, 0));	//设置字体

			m_check[0].CreateCheck(m_hWnd, L"自动跳过验证码", ID_CHECK_AUTOJMP, 5, _TA_VCODE_TOP + _TA_VCODE_ALL_HEIGHT + 5 + 30, 140, 20);
			
			button[0].CreateButton(m_hWnd, L"跳过", ID_BUTTON_JMP, 116, 52, 50, 20);

#ifdef _TA_VCODE_USE_SKIA
			m_vcode_list.setConfig(SkBitmap::kARGB_8888_Config, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT);
			m_vcode_list.allocPixels();
#else
			HDC hdc = GetDC(m_hWnd);
			//创建兼容DC
			//随后建立与屏幕显示兼容的内存显示设备
			m_hVcodeMemDC = CreateCompatibleDC(NULL);
			//下面建立一个与屏幕显示兼容的位图，至于位图的大小嘛，可以用窗口的大小
			m_hVcodeMemBitmap = CreateCompatibleBitmap(hdc, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(m_hVcodeMemDC, m_hVcodeMemBitmap);
			DeleteObject(hOldBitmap);

			::SetBkMode(m_hVcodeMemDC, OPAQUE | TRANSPARENT); //更改背景模式，其中iMode的值为OPAQUE或TRANSPARENT
			//FillSolidRect(m_MemDC, 0, 0, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT, TA_BK_RGB);

			ReleaseDC(m_hWnd, hdc);
#endif
			m_edit_oldWndProc = (WNDPROC)::SetWindowLong(text[0].hWnd(), GWL_WNDPROC, (DWORD)OnEditWndProc);
			
			text[0].SetFocus();

			SetTimer(m_hWnd, ID_TIMER_ANIMATION, kGifVcodeElapse, NULL);

			m_inited = true;
			m_char_select = new CVerifyCodeCharacterSelect;
			m_char_select->CreateCharacterSelect(this);
		}

		int WINAPI CVcodeWindow::VcodeChangeCallback(BOOL bSetInfo)
		{
			/*if (bSetInfo)
			{
				CString szInfo;
				szInfo.Format(_T("%d"), m_queue.GetAccumulateCount());
				m_edtAccumulate.SetWindowText(szInfo);
				return 0;
			}*/

			AutoLock al(m_callback_lock);

			m_queue.Lock();

			COPY_BACKGROUND();

			int count = m_queue.GetCount();
			count = min(_TA_VCODE_COUNT, count);
			if (count > 0)
			{
				SIZE size;

				for (int i=0;
					i < count;
					i++)
				{
					memset(&size, 0, sizeof(SIZE));
					DrawSingleVcode(&canvas, i);
				}
			}
			DrawVcode();

			/*if (m_queue.GetCount() == 0 && snow::conf->g::autohide_vcode)
				ShowWindow(SW_HIDE);*/

			m_queue.UnLock();

			CheckCharacterSelect();

			return 0;
		}

		void CVcodeWindow::JmpVcode()
		{
			if (m_queue.GetCount() > 0)
			{
				text[0].SetText(_T(""));
        m_queue.JumpHead();
				//VcodeChangeCallback();
			}

			text[0].SetFocus();
		}

		void CVcodeWindow::UIUpdateVcode()
		{
			RECT client = {_TA_VCODE_LEFT, _TA_VCODE_TOP, _TA_VCODE_LEFT + _TA_VCODE_WIDTH, _TA_VCODE_TOP + _TA_VCODE_ALL_HEIGHT};
			//::GetClientRect(m_hWnd, &client);
			::InvalidateRect(m_hWnd, &client, TRUE);
		}

    bool CVcodeWindow::IsSkipVcodeType(DWORD dwVcodeType)
    {
      return m_queue.IsSkipType(dwVcodeType);
    }

		bool CVcodeWindow::DrawPicture(TA_REG_QUEUE_INFO *rqi)
		{
			if (!rqi)
				return false;

			int iQueue = m_queue.Add(rqi->bVcodeData, rqi->nLen,
          rqi->dwVcodeType, rqi->iVCodeType, rqi->ocr
#ifdef _TB_FANTASY
					, rqi->unknowvodelen
#endif
				);
			if (iQueue == -1) return false;

			CVerifyCode *pvc = m_queue.GetVerifyCode(iQueue);
			if (!pvc) return false;

      if (iQueue == 0) {
        // first one
        bool isocring = pvc->HasOCR();
        text[0].SetText(isocring ? TEXT_OCRING : L"");
        text[0].Enable(!isocring);
      }
	
			//HDC hdc = ::GetDC(m_hWnd);
			SkCanvas canvas(m_vcode_list);
			DrawSingleVcode(&canvas, iQueue, pvc);

			DrawVcode();	//UIUpdateVcode

			pvc->Wait();

			if (m_queue.IsSkipType(rqi->dwVcodeType))
			{
				rqi->szVcode[0] = 0;
				m_queue.Delete(pvc);
				return false;
			}

			if (pvc->GetVcodeText().length() > 0)
			{
				lstrcpyA(rqi->szVcode, pvc->GetVcodeText().c_str());
			} else {
				rqi->szVcode[0] = 0;
			}

      bool currentInOCR = pvc->HasOCR();
			m_queue.Delete(pvc);

      if (currentInOCR) {
        VcodeChangeCallback(FALSE);
      }

      if (m_queue.GetCount() > 0) {
        bool isocring = m_queue.GetVerifyCode(0)->HasOCR();
        
        text[0].SetText(isocring ? TEXT_OCRING : L"");
        text[0].Enable(!isocring);
      } else {
        text[0].SetText(L"");
      }
			return true;
		}

		void CVcodeWindow::DrawSingleVcode(SkCanvas *canvas, int index, CVerifyCode *pvc)
		{
			SIZE size = {0};
			if (pvc == NULL) {
				pvc = m_queue.GetVerifyCode(index);
				if (pvc == NULL) {
					return;
				}
			}
#ifdef _TA_VCODE_USE_SKIA
      int vt = pvc->GetKind();
      if (IS_VCODE_FRIENDLY(vt)) {
				if (pvc == m_queue.GetVerifyCode(0)) {
					ShowCharacterSelect(pvc);
				}

				size.cx = _TA_VCODE_FRIENDLY_VCODEAREA_SHOW_WIDTH;
				size.cy = _TA_VCODE_FRIENDLY_VCODEAREA_SHOW_HEIGHT;
				pvc->Draw(canvas,
					(_TA_VCODE_WIDTH - size.cx) / 2,
					(index) * _TA_VCODE_WITHBLANK_HEIGHT + (_TA_VCODE_HEIGHT - size.cy) / 2, size.cx, size.cy,
					_TA_VCODE_FRIENDLY_VCODEAREA_WIDTH, _TA_VCODE_FRIENDLY_VCODEAREA_HEIGHT);
			} else {
				m_queue.GetVerifyCodeSize(index, NULL, size);
#ifdef _TB_FANTASY
				if (pvc->IsUnknowLen()) {
					SIZE drawsize = {0};
					if (size.cy > _TA_VCODE_HEIGHT) {
						drawsize.cx = ((float)size.cx / size.cy * _TA_VCODE_HEIGHT);
						if (drawsize.cx > _TA_VCODE_WIDTH) {
							drawsize.cy = ((float)size.cx / _TA_VCODE_WIDTH * size.cy);
							drawsize.cx = _TA_VCODE_WIDTH;
						} else {
							drawsize.cy = _TA_VCODE_HEIGHT;
						}
					}
					pvc->Draw(canvas,
						(_TA_VCODE_WIDTH - drawsize.cx) / 2,
						(index) * _TA_VCODE_WITHBLANK_HEIGHT + (_TA_VCODE_HEIGHT - drawsize.cy) / 2, drawsize.cx, drawsize.cy, size.cx, size.cy);
				} else 
#endif
				{
					pvc->Draw(canvas,
						(_TA_VCODE_WIDTH - size.cx) / 2,
						(index) * _TA_VCODE_WITHBLANK_HEIGHT + (_TA_VCODE_HEIGHT - size.cy) / 2, size.cx, size.cy);
				}
			}
#else
			m_queue.GetVerifyCodeSize(index, m_hVcodeMemDC, size);
			//将图形输出到屏幕上（有点像BitBlt）
			pvc->Draw(m_hVcodeMemDC,
				(_TA_VCODE_WIDTH - size.cx) / 2,
				(index) * _TA_VCODE_WITHBLANK_HEIGHT + (_TA_VCODE_HEIGHT - size.cy) / 2, size.cx, size.cy);
#endif
		}

		void CVcodeWindow::DrawVcode()
		{
			/*HDC hdc = ::GetDC(m_hWnd);
			::BitBlt(hdc, _TA_VCODE_LEFT, _TA_VCODE_TOP, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT, m_hVcodeMemDC, 0, 0, SRCCOPY);
			::ReleaseDC(m_hWnd, hdc);*/
			UIUpdateVcode();
		}

		void CVcodeWindow::NextVcode()
		{
			if (m_queue.GetCount() <= 0) return;

			int nlen = 0;
			/*char szVcode[16];
	
			if (m_queue.GetVerifyCode(0)->GetKind() == _VCODE_SHENSHOU)
			{
				nlen = ::GetWindowTextA(text[0].hWnd(), szVcode, 9);
				szVcode[8] = 0;
			} else {
				nlen = ::GetWindowTextA(text[0].hWnd(), szVcode, 5);
				szVcode[4] = 0;
			}*/

			std::wstring strvcode;
			text[0].GetText(strvcode);
			text[0].SetText(L"");

      int vt = m_queue.GetVerifyCodeKind(0);
      if (IS_VCODE_FRIENDLY(vt)) {
				int indexs = 0;
				if (base::StringToInt(strvcode, &indexs)) {

					std::string input_str;
					int index[4] = {
						(indexs / 1000),
						(indexs % 1000 / 100),
						(indexs % 100 / 10),
						(indexs % 10),
					};

					for (int i = 0; i < 4; i++) {
						int inputIndex = 0; // shift to 1
						for (int j = 0; j < CVerifyCodeCharacterSelect::CHAR_COUNT; j++) {
							if (CVerifyCodeCharacterSelect::id_shift_map[j] == index[i]) {
								inputIndex = j;
                break;
							}
						}
						input_str += CVerifyCodeCharacterSelect::input_str_map[inputIndex];
					}
					m_queue.Finish(0, input_str.c_str());
				} else {
					m_queue.Finish(0, "");
				}
			} else {
				char *szvcode = NULL;
#ifdef NO_UTF8
				lo_W2C(&szvcode, strvcode.c_str());
#else
				lo_W2Utf8(&szvcode, strvcode.c_str());
#endif
				m_queue.Finish(0, szvcode);
				free(szvcode);
			}

			m_dwInputTimes++;
			text[1].SetText(base::UintToString16(m_dwInputTimes).c_str());
		}

		void CVcodeWindow::SkipVcode(DWORD dwVcodeType)
		{
			if (!m_queue.IsSkipType(dwVcodeType))
			{
				m_queue.AddSkipType(dwVcodeType);
			} else {
				return;
			}

			m_queue.FinishType(dwVcodeType);

			/*CString szAccu;
			szAccu.Format(_T("%d"), m_queue.GetAccumulateCount());
			m_edtAccumulate.SetWindowText(szAccu);*/
		}

		void CVcodeWindow::SkipFinish(DWORD dwVcodeType)
		{
			m_queue.DeleteSkipType(dwVcodeType);
			if (m_queue.GetCount() > 0)
			{
				/*for (int i=0;i<_TA_VCODE_COUNT;i++)
				{
					//memcpy(&vqi[i], &vqi[i+1], sizeof(TA_VCODE_QUEUE_INFO));
				}*/
			} else {
/*
		#ifdef _SNOW
				if (snow::conf->g::autohide_vcode) ShowWindow(SW_HIDE);
		#else
				Show(false);
		#endif
*/
			}
		}


		void CVcodeWindow::EnableClose(bool enable_)
		{
			m_enable_close = enable_;
		}

		bool CVcodeWindow::AddVcode(TA_REG_QUEUE_INFO *prqi)
		{
			if (m_jmpall) {
				return false;
			}
			if (m_queue.IsSkipType(prqi->dwVcodeType)) {
				return false;
			}

      //识别的时候不要一直闪
      if (!OCRBase::enable()) {
    show_l:
        if (m_hide) {
          m_hide = false;
          Show();
        } else {
          BOOL bVisible = ::IsWindowVisible(m_hWnd);
          if (!bVisible) Show();
        }
			  SetForegroundWindow(m_hWnd);
      } else if (!m_first_ocr_show) {
        m_first_ocr_show = true;
        goto show_l;
      }

			return DrawPicture(prqi);	//绘验证码
		}

		void CVcodeWindow::doVcodeAnimation()
		{
			bool hasanimate = false;
			CVerifyCode *vc = NULL;
			int count = m_queue.GetCount();
			count = min(_TA_VCODE_COUNT, count);

			for (int i = 0; i < count; i++) {
				vc = m_queue.GetVerifyCode(i);
				if (vc->animateShift()) {
					//all need to shift
					hasanimate = true;
				}
			}

			if (hasanimate) {
				COPY_BACKGROUND();
				if (count > 0) {
					for (int i = 0; i < count; i++) {
						DrawSingleVcode(&canvas, i);
					}
				}
				DrawVcode();
			}
		}

		LRESULT CALLBACK CVcodeWindow::OnEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{	
			//得到旧的过程
			CVcodeWindow *pQueueDlg = (CVcodeWindow *)view::frame::GetWindow(::GetParent(hWnd));
			if (!pQueueDlg)
				return 0;
			//WNDPROC oldWndProc = g_pVCodeDlg->m_oldWndProc;
			WNDPROC oldWndProc = pQueueDlg->m_edit_oldWndProc;
			if (!oldWndProc)
				return 0;
			//按下回车
			//TRACE1("0x%x\r\n", message);
			if (message == WM_KEYUP)
			{
				if (wParam == VK_RETURN)
				{
					if (m_queue.GetCount() > 0) {
#ifdef _TB_FANTASY
						if (m_queue.GetVerifyCode(0)->IsUnknowLen()) {
							pQueueDlg->NextVcode();
						} else
#endif
						/*if (pQueueDlg->m_afterEnter.GetCheck())
						{
							pQueueDlg->NextVcode();
						} else*/ {
							pQueueDlg->JmpVcode();		//按下回车跳过验证码
						}
					}
				} else if (wParam == VK_ESCAPE)
					return 0;
			}
			return oldWndProc(hWnd, message, wParam, lParam);
		}

		LRESULT CALLBACK CVcodeWindow::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
			switch (message)
			{
			case WM_CLOSE:
				if (!m_enable_close)
				{
					HideCharacterSelect();
					Show(false);
					m_hide = true;
					handled = true;
					return FALSE;
				}
				break;
			case WM_SHOWWINDOW:
				/*
				Indicates whether a window is being shown. If wParam is TRUE, the window is being shown. If wParam is FALSE, the window is being hidden.
				*/
				if (wParam) {
					if (m_queue.GetCount() > 0) {
            int vt = m_queue.GetVerifyCodeKind(0);
            if (IS_VCODE_FRIENDLY(vt)) {
							ShowCharacterSelect(NULL);
						}
					}
				}
				break;
			case WM_MOVE:
				{
					int xPos = (int)LOWORD(lParam);
					int yPos = (int)HIWORD(lParam);
					AdjustCharacterSelect(xPos, yPos);
				}
				break;
			case WM_COMMAND:
				
				switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					handled = true;
					switch (LOWORD(wParam))
					{
					case ID_BUTTON_JMP:
						JmpVcode();
						break;
					case ID_CHECK_AUTOJMP:
						if (m_check[0].GetCheck())
						{
							m_jmpall = true;
							m_queue.SkipAll();
						} else {
							m_jmpall = false;
						}
						break;
					}
					break;
				case EN_CHANGE:
					handled = true;
					switch (LOWORD(wParam))
					{
					case ID_TEXT_VCODE:
						if (m_queue.GetCount() > 0)
						{
#ifdef _TB_FANTASY
							if (m_queue.GetVerifyCode(0)->IsUnknowLen()) {
								break;
							}
#endif
							//if (m_afterEnter.GetCheck()) return;
							if (GetWindowTextLengthW(text[0].hWnd()) >= 4)
								//((m_queue.GetVerifyCode(0)->GetKind() == _VCODE_SHENSHOU) ? 8 : 4))	//验证码为4位字符
							{
								NextVcode();
							}
						}
						break;
					}
				}
				return 0;
				break;
			case WM_TIMER:
				switch (LOWORD(wParam)) {
				case ID_TIMER_ANIMATION:
					handled = true;
					if (m_inited) {
						doVcodeAnimation();
					}
					return 0;
					break;
				}
				break;
			case WM_PAINT:
				{
					if (m_first_paint && m_inited)
					{
						m_first_paint = false;
						COPY_BACKGROUND();
					}
					handled = true;
					PAINTSTRUCT ps = {0};
					HDC hdc = BeginPaint(m_hWnd, &ps);
					doSkPaint(hdc, 0, 0, m_width, m_height, 0, 0);
					//::BitBlt(hdc, 0, 0, m_width, m_height, m_hMemDC, 0, 0, SRCCOPY);
#ifdef _TA_VCODE_USE_SKIA
					view::frame::SkiaPaint(hdc, _TA_VCODE_LEFT, _TA_VCODE_TOP, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT,
						m_vcode_list, 0, 0);
#else
					::BitBlt(hdc, _TA_VCODE_LEFT, _TA_VCODE_TOP, _TA_VCODE_WIDTH, _TA_VCODE_ALL_HEIGHT, m_hVcodeMemDC, 0, 0, SRCCOPY);
#endif
					EndPaint(m_hWnd, &ps);
				}
				return 0;
				break;
			}
			return view::frame::CDialog::OnWndProc(message, wParam, lParam, handled);
		}
	}
}