#include "MainWindow.h"

namespace NGINDX
{
    MainWindow::MainWindow()
    {
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::SetMessageHandler(IMessageHandler* messageHandler)
    {
        m_messageHandler = messageHandler;
    }

    LRESULT MainWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_ACTIVATE:
        case WM_INPUT:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_MOUSEHOVER:
        {
            m_messageHandler->OnMouse(message, wParam, lParam);
            break;
        }

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            m_messageHandler->OnKeyboard(message, wParam, lParam);
            break;
        }

        //case WM_MENUCHAR:
        //    // A menu is active and the user presses a key that does not correspond
        //    // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
        //    return MAKELRESULT(0, MNC_CLOSE);

        /*case WM_DISPLAYCHANGE:
            OnDisplayChange();
            break;

        case WM_MOVE:
            OnWindowMoved();
            break;*/

        /*case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
            {
                if (!m_minimized)
                {
                    m_minimized = true;
                    if (!m_inSuspend)
                        OnSuspending();
                    m_inSuspend = true;
                }
            }
            else if (m_minimized)
            {
                m_minimized = false;
                if (m_inSuspend)
                    OnResuming();
                m_inSuspend = false;
            }
            else if (!m_inSizeMove)
            {
                OnWindowSizeChanged(LOWORD(lParam), HIWORD(lParam));
            }
            break;*/

        /*case WM_ENTERSIZEMOVE:
        {
            m_inSizeMove = true;
            break;
        }

        case WM_EXITSIZEMOVE:
        {
            m_inSizeMove = false;
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
            break;
        }*/

        /*case WM_GETMINMAXINFO:
            if (lParam)
            {
                auto info = reinterpret_cast<MINMAXINFO*>(lParam);
                info->ptMinTrackSize.x = 320;
                info->ptMinTrackSize.y = 200;
            }
            break;*/

        /*case WM_ACTIVATEAPP:
            if (wParam)
            {
                m_messageHandler->OnActivated();
            }
            else
            {
                m_messageHandler->OnDeactivated();
            }
            break;*/

        /*case WM_POWERBROADCAST:
        {
            switch (wParam)
            {
            case PBT_APMQUERYSUSPEND:
                if (!m_inSuspend)
                    OnSuspending();
                m_inSuspend = true;
                return TRUE;

            case PBT_APMRESUMESUSPEND:
                if (!m_minimized)
                {
                    if (m_inSuspend)
                        OnResuming();
                    m_inSuspend = false;
                }
                return TRUE;
            }
            break;
        }*/

        case WM_CLOSE:
        {
            DestroyWindow(m_hwnd);
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(m_hwnd, message, wParam, lParam);
        }

        return 0;
    }
}
