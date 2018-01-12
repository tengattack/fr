
#include "frame.h"
#include "frame_window.h"
#include "frame_child.h"

#include "../global.h"
#include "../view.h"

namespace view{
	namespace frame{
		namespace theme{

			HMODULE hUxThemeDll = NULL;
			HMODULE hComctlDll = NULL;
			bool (__stdcall *pIsAppThemed)() = NULL;
			bool (__stdcall *pIsThemeActive)() = NULL;
			bool m_xpstyle = false;

			HRESULT (__stdcall *pSetWindowTheme)(HWND hWnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList) = NULL;
			HANDLE (__stdcall *pOpenThemeData)(HWND hWnd, LPCWSTR pszClassList) = NULL;
			HRESULT (__stdcall *pCloseThemeData)(HANDLE hTheme) = NULL;

			void InitTheme()
			{
				hUxThemeDll = ::LoadLibraryW(L"UxTheme.dll");
				if (hUxThemeDll)
				{
					(FARPROC&)pIsAppThemed = ::GetProcAddress(hUxThemeDll, "IsAppThemed");
					(FARPROC&)pIsThemeActive = ::GetProcAddress(hUxThemeDll,"IsThemeActive");
					(FARPROC&)pOpenThemeData = ::GetProcAddress(hUxThemeDll, "OpenThemeData");
					(FARPROC&)pCloseThemeData = ::GetProcAddress(hUxThemeDll, "CloseThemeData");
					(FARPROC&)pSetWindowTheme = ::GetProcAddress(hUxThemeDll, "SetWindowTheme");

					if (pIsAppThemed != NULL && pIsThemeActive != NULL)
					{
						if (pIsAppThemed() && pIsThemeActive())
						{
							hComctlDll = ::LoadLibraryW(L"comctl32.dll");
							if (hComctlDll)
							{
								DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)::GetProcAddress(hComctlDll, "DllGetVersion");
								if (pDllGetVersion != NULL)
								{
									DLLVERSIONINFO dvi;
									ZeroMemory(&dvi, sizeof(dvi));
									dvi.cbSize = sizeof(dvi);
									HRESULT hRes = pDllGetVersion ((DLLVERSIONINFO *) &dvi);
									if (SUCCEEDED(hRes))
										m_xpstyle = (dvi.dwMajorVersion >= 6);
								}
							}
						}
					}
				}
			}

			bool IsThemeEnabled()
			{
				return m_xpstyle;
			}

			bool EnableWindowTheme(HWND hWnd, LPCWSTR lpszAppName, LPCWSTR lpszClassList, LPCWSTR lpszIdList)
			{
				if (pSetWindowTheme && pOpenThemeData && pCloseThemeData)
				{
						HANDLE hTheme = pOpenThemeData(hWnd, lpszClassList);
						if (hTheme != NULL)
						{
							//VERIFY(pCloseThemeData(theme)==S_OK);
							bool bret = (pSetWindowTheme(hWnd, lpszAppName, lpszIdList) == S_OK);
							pCloseThemeData(hTheme);
							return bret;
						}
				}
				return false;
			}
		};
	};
};
