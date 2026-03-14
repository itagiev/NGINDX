#include "NGINDX/Platform/Windows/MainWindow.h"

namespace NGINDX
{
    MainWindow::MainWindow()
    {
    }

    MainWindow::~MainWindow()
    {
    }

    LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLOSE:
        {
            DestroyWindow(m_hWnd);
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcW(m_hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }
}
