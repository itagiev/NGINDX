#pragma once

#include "NGINDX/Platform/Windows/MainWindow.h"
#include "NGINDX/Core/StepTimer.h"
#include "NGINDX/Core/IMessageHandler.h"

namespace NGINDX
{
    class GameCore : public IMessageHandler
    {
    public:
        static constexpr int g_Width = 960;
        static constexpr int g_Height = 720;

    private:
        MainWindow m_window{};
        bool m_running{ true };

        // bool m_inSizeMove{ false };
        // bool m_inSuspend{ false };
        // bool m_minimized{ false };
        // bool m_fullscreen{ false };

        // Main loop timer.
        NGINDX::StepTimer m_timer{};

#pragma region Input devices

        std::unique_ptr<DirectX::Mouse> m_mouse;
        std::unique_ptr<DirectX::Keyboard> m_keyboard;
        std::unique_ptr<DirectX::GamePad> m_gamePad;

        DirectX::Mouse::State m_mouseState{};
        DirectX::Mouse::ButtonStateTracker m_mouseButtons;
        DirectX::Keyboard::KeyboardStateTracker m_keyboardButtons;
        DirectX::GamePad::ButtonStateTracker m_gamePadButtons;

#pragma endregion

    private:

#pragma region Basic application logic

        // Application initialization.
        void Initialize();
        // Application termination.
        void Terminate();
        // Main loop.
        void Tick();

#pragma region Message handlers

        void OnMouse(UINT message, WPARAM wParam, LPARAM lParam);
        void OnKeyboard(UINT message, WPARAM wParam, LPARAM lParam);
        void OnActivated();
        void OnDeactivated();
        void OnSuspending();
        void OnResuming();
        void OnWindowMoved();
        void OnDisplayChange();
        void OnWindowSizeChanged(int width, int height);

#pragma endregion

#pragma endregion

    protected:
        // This function can be used to initialize application state and will run after essential
        // hardware resources are allocated. Some state that does not depend on these resources
        // should still be initialized in the constructor such as pointers and flags.
        virtual void Startup() = 0;
        virtual void Cleanup() = 0;

        // Decide if you want the app to exit. By default, app continues until the 'ESC' key is pressed.
        virtual bool IsDone();

        // The update method will be invoked once per frame. Both state updating and scene
        // rendering should be handled by this method.
        virtual void Update(const NGINDX::StepTimer& timer) = 0;

        // Official rendering pass
        virtual void Render() = 0;

        // Override this in applications that use DirectX Raytracing to require a DXR-capable device.
        virtual bool RequiresRaytracingSupport() const { return false; }

    public:
        GameCore() = default;
        virtual ~GameCore() = default;

        // GameCore(GameCore&&) = default;
        // virtual GameCore& operator=(GameCore&&) = default;

        // GameCore(GameCore const&) = delete;
        // virtual GameCore& operator=(GameCore const&) = delete;

        int Run(LPCWSTR title, HINSTANCE hInstance, int nCmdShow);
    };
}
