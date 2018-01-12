 
#include "frame.h"
#include "frame_window.h"

#include "../global.h"
#include "../view.h"

#include "skin.h"
#include "skin_windowex.h"

namespace view{
	namespace frame{
		namespace skin{

        /// <summary>
        /// Gets a value indicating if the maximize box needs to be drawn on the specified form.
        /// </summary>
        /// <param name="form">The form to check.</param>
        /// <returns></returns>
        bool WindowEx::IsDrawMaximizeBox(CWindow *pWindow)
        {
#ifdef _DEBUG
			//return true;
#endif
			DWORD dwStyle = ::GetWindowLong(pWindow->hWnd(), GWL_STYLE);
			return (dwStyle & WS_MAXIMIZEBOX);
            /*return form.MaximizeBox && form.FormBorderStyle != FormBorderStyle.SizableToolWindow &&
                   form.FormBorderStyle != FormBorderStyle.FixedToolWindow;*/
        }
        /// <summary>
        /// Gets a value indicating if the minimize box needs to be drawn on the specified form.
        /// </summary>
        /// <param name="form">The form to check .</param>
        /// <returns></returns>
        bool WindowEx::IsDrawMinimizeBox(CWindow *pWindow)
        {
			DWORD dwStyle = ::GetWindowLong(pWindow->hWnd(), GWL_STYLE);
			return (dwStyle & WS_MINIMIZEBOX);
            /*return form.MinimizeBox && form.FormBorderStyle != FormBorderStyle.SizableToolWindow &&
                   form.FormBorderStyle != FormBorderStyle.FixedToolWindow;*/
        }

        /// <summary>
        /// Calculates the border size for the given form.
        /// </summary>
        /// <param name="form">The form.</param>
        /// <returns></returns>
        void WindowEx::GetBorderSize(CWindow *pWindow, SIZE* pSize)
        {
			memset(pSize, 0, sizeof(SIZE));
			//pSize->cx = 2;
            /*Size border = new Size(0, 0);

            // Check for Caption
			DWORD style = ::GetWindowLong(pWindow->hWnd(), GWL_STYLE);
            bool caption = (style & WS_CAPTION) != 0;
            int factor = SystemInformation.BorderMultiplierFactor - 1;

            OperatingSystem system = Environment.OSVersion;
            bool isVista = system.Version.Major >= 6 && VisualStyleInformation.IsEnabledByUser;

            switch (form.FormBorderStyle)
            {
                case FormBorderStyle.FixedToolWindow:
                case FormBorderStyle.FixedSingle:
                case FormBorderStyle.FixedDialog:
                    border = SystemInformation.FixedFrameBorderSize;
                    break;
                case FormBorderStyle.SizableToolWindow:
                case FormBorderStyle.Sizable:
                    if (isVista)
                        border = SystemInformation.FrameBorderSize;
                    else
                        border = SystemInformation.FixedFrameBorderSize +
                            (caption ? SystemInformation.BorderSize + new Size(factor, factor)
                                : new Size(factor, factor));
                    break;
                case FormBorderStyle.Fixed3D:
                    border = SystemInformation.FixedFrameBorderSize + SystemInformation.Border3DSize;
                    break;
            }

            return border;*/
        }

        /// <summary>
        /// Gets the height of the caption.
        /// </summary>
        /// <param name="form">The form.</param>
        /// <returns></returns>
        int WindowEx::GetCaptionHeight(CWindow *pWindow)
        {
            /*return form.FormBorderStyle != FormBorderStyle.SizableToolWindow &&
                   form.FormBorderStyle != FormBorderStyle.FixedToolWindow
                       ? SystemInformation.CaptionHeight + 2
                       : SystemInformation.ToolWindowCaptionHeight + 1;*/
			return CSkinData::m_ci.height;
        }

        /// <summary>
        /// Gets a value indicating whether the given form has a system menu.
        /// </summary>
        /// <param name="form">The form.</param>
        /// <returns></returns>
        bool WindowEx::HasMenu(CWindow *pWindow)
        {
			/*DWORD dwStyle = ::GetWindowLong(pWindow->hWnd(), GWL_STYLE);

            return form.FormBorderStyle == FormBorderStyle.Sizable || form.FormBorderStyle == FormBorderStyle.Fixed3D ||
                    form.FormBorderStyle == FormBorderStyle.FixedSingle;*/
			return true;
        }

        /// <summary>
        /// Gets the screen rect of the given form
        /// </summary>
        /// <param name="form">The form.</param>
        /// <returns></returns>
        void WindowEx::GetScreenRect(CWindow *pWindow, RECT* pRect)
        {
			HWND hParent = pWindow->GetParent();
			if (hParent)
			{
				::GetWindowRect(pWindow->hWnd(), pRect);
				::ClientToScreen(hParent, (LPPOINT)pRect);
				::ClientToScreen(hParent, &(((LPPOINT)pRect)[1]));
			} else {
				::GetWindowRect(pWindow->hWnd(), pRect);
			}
            //return (form.Parent != null) ? form.Parent.RectangleToScreen(form.Bounds) : form.Bounds;
        }
		};
	};
};
