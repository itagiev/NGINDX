#include "GameCore.h"
#include "NGINDX/Graphics/D3D12Backend.h"

namespace NGINDX
{
    int GameCore::Run(LPCWSTR title, HINSTANCE hInstance, int nCmdShow)
    {
        // Проверить поддержку SIMD (SSE и т.п.)
        if (!DirectX::XMVerifyCPUSupport())
            return 1;

        // Без этого не работает gamepad
        /*Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
        if (FAILED(initialize))
            return 1;*/

        // В качестве обработчика событий устанавливается данный экземпляр приложения
        m_window.SetMessageHandler(this);

        // Создание окна
        if (!m_window.Create(hInstance, title, CW_USEDEFAULT, CW_USEDEFAULT, g_Width, g_Height))
        {
            return 1;
        }

        // Инициализация игры
        Initialize();

        // Win32 api команды
        m_window.ShowAndUpdate(nCmdShow);

        // Основной цикл игры, пока окно не закрывается или пока сам игрок не выйдет из игры
        while (!(m_window.Quit() || IsDone()))
        {
            // Сбор событий окна
            m_window.PeekMessages();
            // Кадр
            Tick();
        }

        // Завершение работы приложения, освобождение ресурсов и пр.
        Shutdown();

        // Результат работы - последнее значение wParam из события окна
        return (int)m_window.ExitCode();
    }

    void GameCore::Initialize()
    {
        NGINDX::D3D12::AppDesc appDesc{};
        appDesc.Window = m_window.GetWindow();
        NGINDX::D3D12::Initialize(appDesc);

        // Input devices initialization
        m_mouse = std::make_unique<DirectX::Mouse>();
        m_mouse->SetWindow(m_window.GetWindow());
        m_keyboard = std::make_unique<DirectX::Keyboard>();
        //m_gamePad = std::make_unique<DirectX::GamePad>();

        Startup();
    }

    void GameCore::Shutdown()
    {
        Cleanup();

        NGINDX::D3D12::Shutdown();
    }

    void GameCore::Tick()
    {
        // Получить последние состояния input устройств
        m_mouseState = m_mouse->GetState();
        m_mouseButtons.Update(m_mouseState);

        m_keyboardButtons.Update(m_keyboard->GetState());

        /*const auto pad = m_gamePad->GetState(0);
        if (pad.IsConnected())
        {
            m_gamePadButtons.Update(pad);
        }
        else
        {
            m_gamePadButtons.Reset();
        }*/

        m_timer.Tick([this]() { Update(m_timer); });

        Draw();
    }

    bool GameCore::IsDone()
    {
        return m_keyboardButtons.pressed.Escape;
    }

#pragma region Message handlers

    void GameCore::OnMouse(UINT message, WPARAM wParam, LPARAM lParam)
    {
        DirectX::Mouse::ProcessMessage(message, wParam, lParam);
    }

    void GameCore::OnKeyboard(UINT message, WPARAM wParam, LPARAM lParam)
    {
        DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
    }

    // void GameCore::OnActivated()
    // {
    // }

    // void GameCore::OnDeactivated()
    // {
    // }

    // void GameCore::OnSuspending()
    // {
    // }

    // void GameCore::OnResuming()
    // {
    //     m_timer.ResetElapsedTime();
    // 
    //     m_mouseState = {};
    //     m_mouseButtons.Reset();
    //     m_keyboardButtons.Reset();
    //     m_gamePadButtons.Reset(); 
    // }

    // void GameCore::OnWindowMoved()
    // {
    // }

    // void GameCore::OnDisplayChange()
    // {
    // }

    // void GameCore::OnWindowSizeChanged(int width, int height)
    // {
    // }

#pragma endregion
}
