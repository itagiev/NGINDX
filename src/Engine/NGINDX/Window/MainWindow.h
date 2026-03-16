#pragma once

#include "NGINDX/Window/BaseWindow.h"
#include "NGINDX/Core/IMessageHandler.h"

namespace NGINDX
{
    class MainWindow final : public BaseWindow<MainWindow>
    {
    private:
        IMessageHandler* m_messageHandler{ nullptr };

    public:
        MainWindow();
        ~MainWindow() override;

        void SetMessageHandler(IMessageHandler* messageHandler);
        LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) override;
    };
}
