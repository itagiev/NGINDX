#pragma once

#include "BaseWindow.h"

namespace NGINDX
{
    class MainWindow final : public BaseWindow<MainWindow>
    {
    public:
        MainWindow();
        ~MainWindow() override;

        LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    };
}
