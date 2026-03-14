#include "NGINDX/Core/GameCore.h"
#include "NGINDX/Core/CommandLineArgs.h"
#include "NGINDX/Core/Log.h"

namespace NGINDX
{
    int GameCore::Run(LPCWSTR title, HINSTANCE hInstance, int nCmdShow)
    {
        // Проверить поддержку SIMD (SSE и т.п.)
        if (!DirectX::XMVerifyCPUSupport())
            return 1;

        // Без этого не работает gamepad
        Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
        if (FAILED(initialize))
            return 1;

        // Установка метода на отработку событий окна
        m_window.SetMessageHandler([this](UINT uMsg, WPARAM wParam, LPARAM lParam)->LRESULT
            {
                return HandleMessage(uMsg, wParam, lParam);
            });

        // Правка разрешения (учитывается client size)
        RECT rc{ 0, 0, static_cast<LONG>(g_Width), static_cast<LONG>(g_Height) };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

        // Создание окна
        if (!m_window.Create(hInstance, title, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top))
        {
            return 1;
        }

        // Инициализация игры
        Initialize();

        // Win32 api команды
        m_window.ShowAndUpdate(nCmdShow);

        // Основной цикл игры, пока окно не закрывается или пока сам игрок не выйдет из игры
        while (!m_window.Quit() && !IsDone())
        {
            // Сбор событий окна
            m_window.PeekMessages();
            // Кадр
            Tick();
        }

        // Завершение работы приложения, освобождение ресурсов и пр.
        Terminate();

        // Результат работы - последнее значение wParam из события окна
        return (int)m_window.ExitCode();
    }

    void GameCore::Initialize()
    {
        // Input devices initialization
        m_mouse = std::make_unique<DirectX::Mouse>();
        m_mouse->SetWindow(m_window.GetWindow());
        m_keyboard = std::make_unique<DirectX::Keyboard>();
        m_gamePad = std::make_unique<DirectX::GamePad>();

        Startup();
    }

    void GameCore::Terminate()
    {
        Cleanup();
    }

    void GameCore::Tick()
    {
        // Получить последние состояния input устройств
        m_mouseState = m_mouse->GetState();
        m_mouseButtons.Update(m_mouseState);

        m_keyboardButtons.Update(m_keyboard->GetState());

        const auto pad = m_gamePad->GetState(0);
        if (pad.IsConnected())
        {
            m_gamePadButtons.Update(pad);
        }
        else
        {
            m_gamePadButtons.Reset();
        }

        // TODO: Message handler повторить как этот tick по template-у
        m_timer.Tick([&]()
            {
                Update(m_timer);
            });

        Render();
    }

    bool GameCore::IsDone()
    {
        return m_keyboardButtons.pressed.Escape;
    }

#pragma region Message handlers

    LRESULT GameCore::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
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
            DirectX::Mouse::ProcessMessage(uMsg, wParam, lParam);
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
            break;

        case WM_SYSKEYDOWN:
        {
            DirectX::Keyboard::ProcessMessage(uMsg, wParam, lParam);
            if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
            {
                // Implements the classic ALT+ENTER fullscreen toggle.
                if (m_fullscreen)
                {
                    SetWindowLongPtr(m_window.GetWindow(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
                    SetWindowLongPtr(m_window.GetWindow(), GWL_EXSTYLE, 0);

                    ShowWindow(m_window.GetWindow(), SW_SHOWNORMAL);

                    RECT rc{ 0, 0, static_cast<LONG>(g_Width), static_cast<LONG>(g_Height) };
                    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

                    SetWindowPos(m_window.GetWindow(), HWND_TOP,
                        0, 0, rc.right - rc.left, rc.bottom - rc.top,
                        SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                }
                else
                {
                    SetWindowLongPtr(m_window.GetWindow(), GWL_STYLE, WS_POPUP);
                    SetWindowLongPtr(m_window.GetWindow(), GWL_EXSTYLE, WS_EX_TOPMOST);

                    SetWindowPos(m_window.GetWindow(), HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

                    ShowWindow(m_window.GetWindow(), SW_SHOWMAXIMIZED);
                }

                m_fullscreen = !m_fullscreen;
            }
            break;
        }

        case WM_MENUCHAR:
            // A menu is active and the user presses a key that does not correspond
            // to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
            return MAKELRESULT(0, MNC_CLOSE);

        case WM_DISPLAYCHANGE:
            OnDisplayChange();
            break;

        case WM_MOVE:
            OnWindowMoved();
            break;

        case WM_SIZE:
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
            break;

        case WM_ENTERSIZEMOVE:
        {
            m_inSizeMove = true;
            break;
        }

        case WM_EXITSIZEMOVE:
        {
            m_inSizeMove = false;
            RECT rc;
            GetClientRect(m_window.GetWindow(), &rc);
            OnWindowSizeChanged(rc.right - rc.left, rc.bottom - rc.top);
            break;
        }

        case WM_GETMINMAXINFO:
            if (lParam)
            {
                auto info = reinterpret_cast<MINMAXINFO*>(lParam);
                info->ptMinTrackSize.x = 320;
                info->ptMinTrackSize.y = 200;
            }
            break;

        case WM_ACTIVATEAPP:
            if (wParam)
            {
                OnActivated();
            }
            else
            {
                OnDeactivated();
            }
            break;

        case WM_POWERBROADCAST:
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
        }

        case WM_CLOSE:
        {
            NGINDX_LOG(L"Closing window\n");
            DestroyWindow(m_window.GetWindow());
            break;
        }

        case WM_DESTROY:
        {
            NGINDX_LOG(L"Destroying window\n");
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProc(m_window.GetWindow(), uMsg, wParam, lParam);
        }

        return 0;
    }

    void GameCore::OnActivated()
    {
    }

    void GameCore::OnDeactivated()
    {
    }

    void GameCore::OnSuspending()
    {
    }

    void GameCore::OnResuming()
    {
        m_timer.ResetElapsedTime();

        m_mouseState = {};
        m_mouseButtons.Reset();
        m_keyboardButtons.Reset();
        m_gamePadButtons.Reset(); 
    }

    void GameCore::OnWindowMoved()
    {
    }

    void GameCore::OnDisplayChange()
    {
    }

    void GameCore::OnWindowSizeChanged(int width, int height)
    {
    }

#pragma endregion
}
