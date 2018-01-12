/////////////////////////////////////////////////////////////////////////////
// TrayIcon.h : header file
//

#ifndef _INCLUDED_TRAYICON_H_
#define _INCLUDED_TRAYICON_H_

/////////////////////////////////////////////////////////////////////////////
// CTrayIcon window
#include <string>

class CTrayIcon
{
// Construction/destruction
public:
	CTrayIcon();
	CTrayIcon(HWND hWnd, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID);
	virtual ~CTrayIcon();

	typedef struct tagTANOTIFYICONDATA {
		DWORD cbSize;
		HWND hWnd;
		UINT uID;
		UINT uFlags;
		UINT uCallbackMessage;
		HICON hIcon;
		TCHAR szTip[128];
		DWORD dwState;
		DWORD dwStateMask;
		TCHAR szInfo[256];
		DWORD uTimeoutAndVersion;
		TCHAR szInfoTitle[64];
		DWORD dwInfoFlags;
		GUID guidItem;
	} TANOTIFYICONDATA, *PTANOTIFYICONDATA;

	DWORD m_uLastTimeOut;
// Operations
public:
	BOOL Enabled() { return m_bEnabled; }
	BOOL Visible() { return !m_bHidden; }

	//Create the tray icon
	int Create(HWND hWnd, UINT uCallbackMessage, LPCTSTR szTip, HICON icon, UINT uID, BOOL bIsNotify = FALSE, LPCTSTR szWindowTitle = NULL);

	//Change or retrieve the Tooltip text
	BOOL    SetTooltipText(LPCTSTR pszTooltipText,DWORD uTimeOut);
	//BOOL    SetTooltipText(UINT nID,DWORD uTimeOut);
	void	GetTooltipText(std::wstring& text);

	//Change or retrieve the icon displayed
	BOOL  SetIcon(HICON hIcon);
	BOOL  SetIcon(LPCTSTR lpIconName);
	BOOL  SetIcon(UINT nIDResource);
	BOOL  SetStandardIcon(LPCTSTR lpIconName);
	BOOL  SetStandardIcon(UINT nIDResource);
	HICON GetIcon() const;
	void  HideIcon();
	void  ShowIcon();
	void  RemoveIcon();
	void  MoveToRight();

	//Change or retrieve the window to send notification messages to
	BOOL  SetNotificationWnd(HWND pNotifyWnd);
	BOOL  Ta_Shell_NotifyIcon(DWORD dwMessage, PTANOTIFYICONDATA lpData);
	HWND GetNotificationWnd() const;

	//Default handler for tray notification message
	virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTrayIcon)
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL			m_bEnabled;	// does O/S support tray icon?
	BOOL			m_bHidden;	// Has the icon been hidden?
	BOOL			m_bNotify;  // Has Notify style?
	UINT			m_MenuID;
	HWND			m_hWnd;
	TANOTIFYICONDATA	m_tnd;
	HANDLE			m_hTimerThread;
	static DWORD WINAPI TimerThread(LPVOID lparam);
	//DECLARE_DYNAMIC(CTrayIcon)
};


#endif

/////////////////////////////////////////////////////////////////////////////
