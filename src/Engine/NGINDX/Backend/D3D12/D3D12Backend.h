#pragma once

// Link necessary d3d12 libraries.
//#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace NGINDX::D3D12
{
    // 1. Create the ID3D12Device using the D3D12CreateDevice function.
    // 2. Create an ID3D12Fence object and query descriptor sizes.
    // 3. Check 4X MSAA quality level support.
    // 4. Create the command queue, command list allocator, and main command list.
    // 5. Describe and create the swap chain.
    // 6. Create the descriptor heaps the application requires.
    // 7. Resize the back buffer and create a render target view to the back buffer.
    // 8. Create the depth / stencil buffer and its associated depth / stencil view.
    // 9. Set the viewport and scissor rectangles.

    // Helpers
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception();
        }
    }

    // Methods
    void Initialize();

    void Shutdown();
}
