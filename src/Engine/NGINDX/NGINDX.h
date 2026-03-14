#pragma once

#include "NGINDX/Core/GameCore.h"
#include <memory>

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
