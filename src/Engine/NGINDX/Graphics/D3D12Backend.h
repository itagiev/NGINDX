#pragma once

#include "D3D12Utils.h"

// Link necessary d3d12 libraries.
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace NGINDX::D3D12
{
    struct AppDesc
    {
        D3D_DRIVER_TYPE D3DDriverType{ D3D_DRIVER_TYPE_HARDWARE };
        DXGI_FORMAT BackBufferFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
        DXGI_FORMAT DepthStencilFormat{ DXGI_FORMAT_D24_UNORM_S8_UINT };
        int ClientWidth{ 960 };
        int ClientHeight{ 720 };
        DXGI_RATIONAL RefreshRate{ 60, 1 };
        HWND Window{ nullptr };
    };

    // 1. Create the ID3D12Device using the D3D12CreateDevice function.
    // 2. Create an ID3D12Fence object and query descriptor sizes.
    // 3. Check 4X MSAA quality level support.
    // 4. Create the command queue, command list allocator, and main command list.
    // 5. Describe and create the swap chain.
    // 6. Create the descriptor heaps the application requires.
    // 7. Resize the back buffer and create a render target view to the back buffer.
    // 8. Create the depth / stencil buffer and its associated depth / stencil view.
    // 9. Set the viewport and scissor rectangles.

    void Initialize(const AppDesc& desc);
    void Shutdown();

    void OnResize();
    void Draw();
}
