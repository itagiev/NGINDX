#pragma once

namespace NGINDX
{
    template<typename TWindow>
    class BaseWindow
    {
    private:
        bool m_quit{ false };
        WPARAM m_exitCode{ 0 };

    protected:
        HWND m_hwnd{ NULL };

        virtual LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) = 0;

    public:
        static inline LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            BaseWindow* pThis{ nullptr };

            if (message == WM_NCCREATE)
            {
                CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
                pThis = static_cast<BaseWindow*>(pCreate->lpCreateParams);
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

                pThis->m_hwnd = hwnd;
            }
            else
            {
                pThis = reinterpret_cast<BaseWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            }

            if (pThis)
            {
                return pThis->HandleMessage(message, wParam, lParam);
            }
            else
            {
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }

        BaseWindow() = default;
        virtual ~BaseWindow() = default;

        inline BOOL Create(HINSTANCE hInstance,
            LPCWSTR title,
            int x, int y, int width, int height,
            DWORD dwStyle = WS_OVERLAPPEDWINDOW,
            DWORD dwExStyle = 0)
        {
            WNDCLASSEXW wcex{ 0 };

            wcex.cbSize = sizeof(WNDCLASSEXW);
            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = TWindow::WindowProc;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = hInstance;
            wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
            wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wcex.lpszMenuName = nullptr;
            wcex.lpszClassName = title;
            wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

            RegisterClassEx(&wcex);

            RECT rc{ static_cast<LONG>(0), static_cast<LONG>(0), static_cast<LONG>(width), static_cast<LONG>(height) };
            AdjustWindowRect(&rc, dwStyle, FALSE);

            m_hwnd = CreateWindowEx(dwExStyle,
                title,
                title,
                dwStyle,
                x, y, rc.right - rc.left, rc.bottom - rc.top,
                nullptr, nullptr,
                hInstance,
                this);

            if (!m_hwnd)
            {
                return FALSE;
            }

            return TRUE;
        }

        inline void ShowAndUpdate(int nCmdShow)
        {
            ShowWindow(m_hwnd, nCmdShow);
            UpdateWindow(m_hwnd);
        }

        inline void PeekMessages()
        {
            MSG msg{};
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    m_quit = true;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            m_exitCode = msg.wParam;
        }

        inline HWND GetWindow() const { return m_hwnd; }
        inline bool Quit() const { return m_quit; }
        inline WPARAM ExitCode() const { return m_exitCode; }
    };
}
