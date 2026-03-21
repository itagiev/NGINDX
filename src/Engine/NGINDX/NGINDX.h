#pragma once

#include "NGINDX/Core/Utils.h"
#include "NGINDX/Graphics/D3D12Backend.h"

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

    try
    {
        std::unique_ptr<NGINDX::GameCore> app{ CreateApplication() };
        return app->Run(Title(), hInstance, nCmdShow);
    }
    catch (const NGINDX::wexception& ex)
    {
        MessageBox(nullptr, ex.wwhat().c_str(), L"Failure", MB_OK);
        return 0;
    }
    catch (...)
    {
        MessageBox(nullptr, L"", L"Failure", MB_OK);
        return 0;
    }
}
