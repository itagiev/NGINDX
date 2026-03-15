#pragma once

namespace NGINDX
{
    class IMessageHandler
    {
    public:
        IMessageHandler() = default;
        virtual ~IMessageHandler() = default;

        virtual void OnMouse(UINT message, WPARAM wParam, LPARAM lParam) = 0;
        virtual void OnKeyboard(UINT message, WPARAM wParam, LPARAM lParam) = 0;
        // virtual void OnActivated() = 0;
        // virtual void OnDeactivated() = 0;
        // virtual void OnSuspending() = 0;
        // virtual void OnResuming() = 0;
        // virtual void OnWindowMoved() = 0;
        // virtual void OnDisplayChange() = 0;
        // virtual void OnWindowSizeChanged(int width, int height) = 0;
    };
}
