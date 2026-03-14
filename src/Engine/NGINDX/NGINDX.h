#pragma once

#include <memory>

#include "NGINDX/Platform/Windows/MainWindow.h"
#include "NGINDX/Core/CommandLineArgs.h"
#include "NGINDX/Core/Log.h"
#include "NGINDX/Core/StepTimer.h"
#include "NGINDX/Core/GameCore.h"

extern LPCWSTR Title();
extern NGINDX::GameCore* CreateApplication();

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE /*hPrevInstance*/,
    _In_ LPWSTR /*lpCmdLine*/,
    _In_ int nCmdShow)
{
    std::unique_ptr<NGINDX::GameCore> app{ CreateApplication() };
    return app->Run(Title(), hInstance, nCmdShow);
}
