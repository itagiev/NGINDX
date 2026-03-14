#pragma once

#include "NGINDX/Platform/Windows/MainWindow.h"

class NGINDX::MainWindow;

namespace NGINDX
{
    class GameCore
    {
    private:
        MainWindow m_window{};
        bool m_running{ true };

    protected:
        // This function can be used to initialize application state and will run after essential
        // hardware resources are allocated. Some state that does not depend on these resources
        // should still be initialized in the constructor such as pointers and flags.
        virtual void Startup() = 0;
        virtual void Cleanup() = 0;

        // Decide if you want the app to exit. By default, app continues until the 'ESC' key is pressed.
        //virtual bool IsDone();

        // The update method will be invoked once per frame. Both state updating and scene
        // rendering should be handled by this method.
        virtual void Update(float deltaTime) = 0;

        // Official rendering pass
        virtual void Render() = 0;

        // Override this in applications that use DirectX Raytracing to require a DXR-capable device.
        virtual bool RequiresRaytracingSupport() const { return false; }

    protected:
        void InitializeApplication();
        void TerminateApplication();
        void UpdateApplication();

        LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    public:
        GameCore();
        virtual ~GameCore();

        int Run(LPCWSTR title, HINSTANCE hInstance, int nCmdShow);
    };
}
