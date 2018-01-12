

#ifndef _SNOW_CUTE_BASE_TITLETAB_H_
#define _SNOW_CUTE_BASE_TITLETAB_H_ 1

#include "../common.h"
#include "../frame/frame_child.h"
#include "../frame/frame_custom.h"

#define TT_BASE				0x20
#define TT_SELCHANGE		(TT_BASE + 1)

namespace view{
	namespace frame{

		enum TitleTabState {
			kTTSNon = 0,
			kTTSHover,
			kTTSDown,
			kTTSSelect
		};

		static const wchar_t kTitleTabClassName[] = L"taTitleTab";
		typedef struct _TA_TITLE_TAB_ITEM{
			std::wstring text;
			int width;
			int left;
			int fade;
			TitleTabState state;
			TitleTabState prevstate;
		} TA_TITLE_TAB_ITEM, *PTA_TITLE_TAB_ITEM;

		class CTitleTab : public CCustom{

		public:
			CTitleTab();
			~CTitleTab();

			static bool RegisterClass();

			bool CreateTitleTab(HWND hParent, LPWSTR lpszText, unsigned short id, 
				int x, int y, int width, int height, DWORD dwStyle = WS_CHILD | WS_TABSTOP);

			virtual int OnMouseMove(bool movein, int x, int y);
			virtual int OnMouseLeave();

			virtual LRESULT CALLBACK OnWndProc(UINT message, WPARAM wParam, LPARAM lParam, bool& handled);
			virtual WindowType GetType();

			//funciton
			int AddTab(LPCWSTR lpszText, int width = -1);
			void DeleteTab(int iIndex);
			void SetCurSel(int iIndex);
			int GetCurSel();

			void AllowNonSelect(bool allow = true);

		protected:
			std::vector<TA_TITLE_TAB_ITEM> m_tab;

			int m_hovered_;
			int m_selected_;
			int m_downed_;
			bool m_lbuttondown, m_lbuttonup;
			bool m_allow_nselect;

			virtual void UIUpdate(bool force_ = false);
			virtual void DrawTabs(int* pIndexs, int count, HDC hdc = NULL);
			virtual void TimerMessages(int* pIndexs, int count);

			virtual int XHitTest(int x);
			virtual void SelChange(int iSelIndex);
			virtual void SetState(int iNewIndex, TitleTabState state);
			
			void DrawSelectTab(SkCanvas& canvas, TA_TITLE_TAB_ITEM *pItem, BYTE alpha, int iIndex);
			void DrawHoverTab(SkCanvas& canvas, TA_TITLE_TAB_ITEM *pItem, BYTE alpha, int iIndex);
			void DrawDownTab(SkCanvas& canvas, TA_TITLE_TAB_ITEM *pItem, BYTE alpha, int iIndex);
		};
	};
};

#endif