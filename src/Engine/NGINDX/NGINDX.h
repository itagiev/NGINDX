#pragma once

#include "NGINDX/Core/GameCore.h"

extern LPCWSTR Title();
extern NGINDX::GameCore* CreateApplication();

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE /*hPrevInstance*/,
    _In_ LPWSTR /*lpCmdLine*/,
    _In_ int nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    std::unique_ptr<NGINDX::GameCore> app{ CreateApplication() };
    return app->Run(Title(), hInstance, nCmdShow);
}
