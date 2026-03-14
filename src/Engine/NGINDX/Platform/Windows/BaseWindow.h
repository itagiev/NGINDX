#pragma once

namespace NGINDX
{
    template<typename TWindow>
    class BaseWindow
    {
    private:
        bool m_quit{ false };
        WPARAM m_exitCode{ 0 };
        std::function<LRESULT(UINT, WPARAM, LPARAM)> m_messageHandler{ nullptr };

    protected:
        HWND m_hWnd{ NULL };

        virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    public:
        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            BaseWindow* pThis{ nullptr };

            if (uMsg == WM_NCCREATE)
            {
                CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
                pThis = static_cast<BaseWindow*>(pCreate->lpCreateParams);
                SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));

                pThis->m_hWnd = hWnd;
            }
            else
            {
                pThis = reinterpret_cast<BaseWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
            }

            if (pThis)
            {
                return pThis->m_messageHandler(uMsg, wParam, lParam);
            }
            else
            {
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
        }

        BaseWindow()
        {
        }

        virtual ~BaseWindow() = default;

        BOOL Create(HINSTANCE hInstance,
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

            m_hWnd = CreateWindowEx(dwExStyle,
                title,
                title,
                dwStyle,
                x, y, width, height,
                nullptr, nullptr,
                hInstance,
                this);

            if (!m_hWnd)
            {
                return FALSE;
            }

            return TRUE;
        }

        void PeekMessages()
        {
            MSG msg{};
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    m_quit = true;
                    m_exitCode = msg.wParam;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        void SetMessageHandler(std::function<LRESULT(UINT, WPARAM, LPARAM)> callback)
        {
            m_messageHandler = callback;
        }

        bool Quit() const { return m_quit; }

        WPARAM ExitCode() const { return m_exitCode; }

        HWND GetWindow() const { return m_hWnd; }
    };
}
