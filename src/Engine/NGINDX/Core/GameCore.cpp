#include "GameCore.h"
#include "CommandLineArgs.h"

namespace NGINDX
{
    GameCore::GameCore()
    {
    }

    GameCore::~GameCore()
    {
    }

    /*bool GameCore::IsDone(void)
    {
        return false;
    }*/

    void GameCore::InitializeApplication()
    {
        Startup();
    }

    void GameCore::TerminateApplication()
    {
        Cleanup();
    }

    void GameCore::UpdateApplication()
    {
        Update(0.0f);
        Render();

        //return !IsDone();
    }

    int GameCore::Run(LPCWSTR title, HINSTANCE hInstance, int nCmdShow)
    {
        m_window.SetMessageHandler([this](UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT
            {
                return HandleMessage(uMsg, wParam, lParam);
            });

        RECT rc{ 0, 0, static_cast<LONG>(960), static_cast<LONG>(720) };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        m_window.Create(hInstance, title,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top);

        InitializeApplication();

        ShowWindow(m_window.GetWindow(), nCmdShow);
        UpdateWindow(m_window.GetWindow());

        while (!m_window.Quit())
        {
            m_window.PeekMessages();
            UpdateApplication();
        }

        TerminateApplication();

        return m_window.ExitCode();
    }

    LRESULT GameCore::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLOSE:
        {
            DestroyWindow(m_window.GetWindow());
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcW(m_window.GetWindow(), uMsg, wParam, lParam);
        }

        return 0;
    }
}
