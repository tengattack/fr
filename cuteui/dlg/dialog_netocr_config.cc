
#include "stdafx.h"

#include <shellapi.h>

#include "base/global.h"
#include "base/common.h"
#include "base/view.h"
#include "base/frame/frame_window.h"
#include "base/frame/dialog.h"

#include "base/string/stringprintf.h"
#include "base/string/string_number_conversions.h"
#include "base/file/file.h"
#include "base/file/filedata.h"

#include <common/strconv.h>
#include <filecommon/file_path.h>

#include <ocr/ocr_base.h>

#ifdef SNOW
#include "../core/snow_core.h"
#include "../common/snow_common_text_list.h"
#else
#define VIEW_NUMBER_TEXT_WIDTH 80
#endif

#include "dialog_netocr_config.h"

enum _ID_CONTROL {
	ID_BUTTON_OK = 1001,

	ID_BUTTON_SAVE,
	ID_BUTTON_CANCEL,
	ID_BUTTON_TEST,

	ID_STATIC_TEXT,

	ID_TEXT_USERNAME,
	ID_TEXT_PASSWORD,

	ID_TEXT_TIMEOUT,
  ID_TEXT_MAXTHREADS,

  ID_CHECK_ENABLE,

	ID_COMBOBOX_PLATFORM,
  ID_LINK_PLATFORM,
};

#define NETOCR_STATIC_TEXT_WIDTH 100
#define NETOCR_START_LEFT	50
#define NETOCR_OBJECT_LEFT	150
#define NETOCR_OBJECT_WIDTH	200

namespace view {
	namespace frame {

    static const wchar_t *sz_netocr_platform_name[kSNOCRCount] = {
			L"若快打码 (ruokuai.com)",
		};
    static const wchar_t *sz_netocr_platform_url[kSNOCRCount] = {
      L"http://ruokuai.com",
    };

		#define ADD_NETOCR_PLATFORM_TO_COMBOBOX(combo) \
      for (int i = 0; i < (sizeof(sz_netocr_platform_name) / sizeof(const wchar_t *)); i++) { \
        (combo).AddString(sz_netocr_platform_name[i]); \
			} \
			(combo).SetCurSel(0);

		CSnowDialogNetOCRConfig::CSnowDialogNetOCRConfig(HWND hParent)
			: m_wnd_parent(hParent)
			, m_working(false)
      , m_bInit(false)
		{
		}

		CSnowDialogNetOCRConfig::~CSnowDialogNetOCRConfig()
		{
		}

		void CSnowDialogNetOCRConfig::InitWindow()
		{
			int height_= 16, max_width = GetInitialWidth() - 16;

      AddStaticText(ID_STATIC_TEXT, NETOCR_START_LEFT, height_, NETOCR_STATIC_TEXT_WIDTH, 20, L"打码平台:");
      m_combo[0].CreateComboBox(m_hWnd, L"", ID_COMBOBOX_PLATFORM, NETOCR_OBJECT_LEFT, height_, NETOCR_OBJECT_WIDTH, 25);
      ADD_NETOCR_PLATFORM_TO_COMBOBOX(m_combo[0]);
      m_link[0].CreateLink(m_hWnd, L"注册", ID_LINK_PLATFORM, NETOCR_OBJECT_LEFT + NETOCR_OBJECT_WIDTH + 5, height_, 50, 25);
      

      height_ += 25 + 5;
      AddStaticText(ID_STATIC_TEXT, NETOCR_START_LEFT, height_, NETOCR_STATIC_TEXT_WIDTH, 20, L"用户名:");

      m_text[0].CreateText(m_hWnd, L"", ID_TEXT_USERNAME, NETOCR_OBJECT_LEFT, height_, NETOCR_OBJECT_WIDTH, 25,
				WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL);

      height_ += 25 + 5;
      AddStaticText(ID_STATIC_TEXT, NETOCR_START_LEFT, height_, NETOCR_STATIC_TEXT_WIDTH, 20, L"密码:");
      m_text[1].CreateText(m_hWnd, L"", ID_TEXT_PASSWORD, NETOCR_OBJECT_LEFT, height_, NETOCR_OBJECT_WIDTH, 25,
        WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_PASSWORD);

      height_ += 25 + 5;
      AddStaticText(ID_STATIC_TEXT, NETOCR_START_LEFT, height_, NETOCR_STATIC_TEXT_WIDTH, 20, L"超时(s):");
      m_text[2].CreateText(m_hWnd, L"90", ID_TEXT_TIMEOUT, NETOCR_OBJECT_LEFT, height_, VIEW_NUMBER_TEXT_WIDTH, 25,
        WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_NUMBER | ES_CENTER);

      height_ += 25 + 5;
      AddStaticText(ID_STATIC_TEXT, NETOCR_START_LEFT, height_, NETOCR_STATIC_TEXT_WIDTH, 20, L"最大线程数:");
      m_text[3].CreateText(m_hWnd, L"5", ID_TEXT_MAXTHREADS, NETOCR_OBJECT_LEFT, height_, VIEW_NUMBER_TEXT_WIDTH, 25,
        WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL | ES_NUMBER | ES_CENTER);

      height_ += 25 + 5;
      m_check[0].CreateCheck(m_hWnd, L"启用", ID_CHECK_ENABLE, NETOCR_OBJECT_LEFT, height_, NETOCR_OBJECT_WIDTH, 20);

      height_ += 20 + 5 + 10;
      
      m_button[0].CreateButton(m_hWnd,
#ifdef SNOW
        SC_TEXT(Save),
#else
        L"保存",
#endif
        ID_BUTTON_SAVE, max_width - ((80 + 5) * 2), height_, 80, 25, WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_DEFPUSHBUTTON);
      m_button[1].CreateButton(m_hWnd,
#ifdef SNOW
        SC_TEXT(Cancel),
#else
        L"取消",
#endif
        ID_BUTTON_CANCEL, max_width - (80 + 5), height_, 80, 25);
      m_button[2].CreateButton(m_hWnd, L"测试", ID_BUTTON_TEST, max_width - ((80 + 5) * 3), height_, 80, 25);

      height_ += 70;
			SetWindowPos(NULL, 0, 0, GetInitialWidth(), height_, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOZORDER | SWP_NOMOVE);

			LoadConfig();

      m_bInit = true;
		}

		void CSnowDialogNetOCRConfig::LoadConfig()
		{
      int platform = *(int *)OCRBase::getValue(OCRBase::kOCRKPlatform);
      if (platform < 0 || platform >= kSNOCRCount) {
        platform = 0;
      }
      m_combo[0].SetCurSel(platform);

      m_text[0].SetText((LPCWSTR)OCRBase::getValue(OCRBase::kOCRKUsername));
      m_text[1].SetText((LPCWSTR)OCRBase::getValue(OCRBase::kOCRKPassword));

      int timeout = *(int *)OCRBase::getValue(OCRBase::kOCRKTimeout);
      if (timeout <= 0) {
        timeout = 90;
      }
      m_text[2].SetText(base::IntToString16(timeout).c_str());

      int max_threads = *(int *)OCRBase::getValue(OCRBase::kOCRKMaxThreads);
      if (max_threads <= 0) {
        max_threads = 90;
      }
      m_text[3].SetText(base::IntToString16(max_threads).c_str());

      m_check[0].SetCheck(*(bool *)OCRBase::getValue(OCRBase::kOCRKEnable));

			CheckControlEnable();
		}

		void CSnowDialogNetOCRConfig::SaveConfig()
		{
      OCRBase::save();
      CheckControlEnable();
		}

		LRESULT CALLBACK CSnowDialogNetOCRConfig::OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled)
		{
      if (!m_bInit) {
        return CDialog::OnWndProc(message, wParam, lParam, handled);
      }

			switch (message)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == BN_CLICKED)
				{
					handled = true;
					switch (LOWORD(wParam))
					{
					case IDOK:
					case ID_BUTTON_SAVE:
						SaveConfig();
						if (!m_working) {
							Close();
						}
						break;
					case ID_BUTTON_CANCEL:
						if (!m_working) {
              OCRBase::undirty();
							Close();
						}
						break;
					case ID_BUTTON_TEST:
						doWork();
						break;
          case ID_LINK_PLATFORM:
            {
              int iplatform = 0;
              iplatform = m_combo[0].GetCurSel();
              if (iplatform >= 0 && iplatform < kSNOCRCount) {
                ::ShellExecuteW(m_hWnd, L"open", sz_netocr_platform_url[iplatform], L"", L"", SW_SHOW);
              }
            }
            break;
          case ID_CHECK_ENABLE:
            {
              bool benable = m_check[0].GetCheck();
              OCRBase::setValue(OCRBase::kOCRKEnable, &benable);
              CheckControlEnable();
            }
            break;
					}
				} else if (HIWORD(wParam) == CBN_SELCHANGE) {
					switch (LOWORD(wParam)) {
          case ID_COMBOBOX_PLATFORM:
						handled = true;
            int iplatform = 0;
            iplatform = m_combo[0].GetCurSel();
            if (iplatform >= 0 && iplatform < kSNOCRCount) {
              OCRBase::setValue(OCRBase::kOCRKPlatform, &iplatform);
            }
						CheckControlEnable();
						break;
          }
        } else if (HIWORD(wParam) == EN_CHANGE) {
          handled = true;
          std::wstring _text;
          switch (LOWORD(wParam)) {
          case ID_TEXT_USERNAME:
            m_text[0].GetText(_text);
            OCRBase::setValue(OCRBase::kOCRKUsername, _text.c_str());
            break;
          case ID_TEXT_PASSWORD:
            m_text[1].GetText(_text);
            OCRBase::setValue(OCRBase::kOCRKPassword, _text.c_str());
            break;
          case ID_TEXT_TIMEOUT:
            m_text[2].GetText(_text);
            {
              int timeout = 0;
              base::StringToInt(_text, &timeout);
              if (timeout <= 0) {
                timeout = 1;
                m_text[2].SetText(base::IntToString16(timeout).c_str());
              }
              OCRBase::setValue(OCRBase::kOCRKTimeout, &timeout);
            }
            break;
          case ID_TEXT_MAXTHREADS:
            m_text[3].GetText(_text);
            {
              int max_threads = 0;
              base::StringToInt(_text, &max_threads);
              if (max_threads <= 0) {
                max_threads = 1;
                m_text[2].SetText(base::IntToString16(max_threads).c_str());
              }
              OCRBase::setValue(OCRBase::kOCRKMaxThreads, &max_threads);
            }
            break;
          }
          CheckControlEnable();
          break;
        }
				return 0;
			case WM_CLOSE:
				if (m_working) {
					handled = true;
					MsgBoxInfo(L"正在测试中！");
					return 0;
				}
				break;
			}

			return CDialog::OnWndProc(message, wParam, lParam, handled);
		}

    bool CSnowDialogNetOCRConfig::doNetOCRConfig()
		{
			if (CreateDialog(m_wnd_parent, L"打码平台设置", true)) {
				DoModal();
				return true;
			}
			return false;
		}

		void CSnowDialogNetOCRConfig::CheckControlEnable()
		{
      bool bdirty = OCRBase::dirty();
      bool benable = m_check[0].GetCheck();
      m_button[0].Enable(bdirty);
      if (benable) {
        m_button[2].Enable(m_working ? false : (!bdirty));
      } else {
        m_button[2].Enable(false);
      }
		}

		void CSnowDialogNetOCRConfig::doWork()
		{
			if (m_working) {
				return;
			}
      HANDLE hThread = ::CreateThread(NULL, 0, OCRTestProc, this, NULL, NULL);
      if (hThread) {
        CloseHandle(hThread);

        m_working = true;
        m_button[2].Enable(false);
      }
		}

    DWORD WINAPI CSnowDialogNetOCRConfig::OCRTestProc(LPVOID lParam)
		{
      CSnowDialogNetOCRConfig *dlg = (CSnowDialogNetOCRConfig *)lParam;
      if (!dlg) {
        return 0;
      }

      std::wstring path = global::wpath;
      path += L"data/vcode-sample.jpg";

      base::CFile file;
      if (file.Open(base::kFileRead, path.c_str())) {
        base::CFileData fd;
        if (fd.Read(file)) {
          OCRBase *ocr = OCRBase::create();
          if (ocr) {
            std::string ocr_result;
            char *ret_info = NULL;
            OCRBase::OCRResultCode ocr_code = ocr->detect(OCRBase::kOCRCT4Chinese, fd.GetData(), fd.GetSize(), &ocr_result, &ret_info);
            ocr->destory();

            std::wstring info;
            wchar_t *wtext = NULL;
            if (ocr_code == OCRBase::kOCRSuccess) {
              lo_Utf82W(&wtext, ocr_result.c_str());
              base::SStringPrintf(&info, L"成功返回打码结果！打码结果为：「%s」", wtext ? wtext : L"");
              dlg->MsgBoxInfo(info.c_str());
            } else {
              base::SStringPrintf(&info, L"错误！错误代码：%d", ocr_code);
              if (ret_info) {
                lo_Utf82W(&wtext, ret_info);
                if (wtext) {
                  info += L"\r\n";
                  info += wtext;
                }
              }
              
              dlg->MsgBoxError(info.c_str());
            }
            if (wtext) free(wtext);
            if (ret_info) free(ret_info);
          }
        }
      }

      dlg->m_working = false;
      dlg->CheckControlEnable();
			return 0;
		}
	}
}