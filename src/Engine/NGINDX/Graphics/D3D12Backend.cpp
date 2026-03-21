#include "D3D12Backend.h"

namespace NGINDX::D3D12
{
    // Usings
    using Microsoft::WRL::ComPtr;
    using namespace DirectX;

    // Members
    ComPtr<IDXGIFactory4> m_dxgiFactory;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D12Device> m_d3dDevice;                       // Represents a display adapter
    ComPtr<ID3D12Fence> m_fence;                            // Synchronize the GPU and CPU
    UINT64 m_currFence{ 0 };

    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12CommandAllocator> m_directCmdListAlloc;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    UINT m_cbvSrvUavDescriptorSize;     // CBV/SRV/UAV descriptors describe constant buffers, shader resources and unordered access view resources
    UINT m_samplerDescriptorSize;       // Sampler descriptors describe sampler resources (used in texturing)
    UINT m_rtvDescriptorSize;           // RTV descriptors describe render target resources
    UINT m_dsvDescriptorSize;           // DSV descriptors describe depth/stencil resources

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;     // RTV descriptors describe render target resources
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;     // DSV descriptors describe depth/stencil resources

    constexpr int SwapChainBufferCount{ 2 };
    int m_currBackBuffer{ 0 };
    ComPtr<ID3D12Resource> m_swapChainBuffer[SwapChainBufferCount];
    ComPtr<ID3D12Resource> m_depthStencilBuffer;

    D3D12_VIEWPORT m_screenViewport;
    D3D12_RECT m_scissorRect;

    //bool m_4xMsaaState = true;      // 4X MSAA enabled
    //UINT m_4xMsaaQuality = 0;       // quality level of 4X MSAA

    // App settings
    D3D_DRIVER_TYPE m_d3dDriverType;
    DXGI_FORMAT m_backBufferFormat;
    DXGI_FORMAT m_depthStencilFormat;
    int m_clientWidth;
    int m_clientHeight;
    DXGI_RATIONAL m_refreshRate;
    HWND m_window;

    constexpr XMVECTORF32 ClearColor = { 0.15f, 0.15f, 0.15f, 1.0f };

    // Forward declaration
    static void LogAdapters();
    static void LogAdapterOutputs(IDXGIAdapter* adapter);
    static void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

    void CreateCommandObjects();
    void CreateSwapChain();
    void CreateRtvAndDsvDescriptorHeaps();
    ID3D12Resource* CurrentBackBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView();
    void FlushCommandQueue();


    // Methods
    void Initialize(const AppDesc& desc)
    {
        m_d3dDriverType = desc.D3DDriverType;
        m_backBufferFormat = desc.BackBufferFormat;
        m_depthStencilFormat = desc.DepthStencilFormat;
        m_clientWidth = desc.ClientWidth;
        m_clientHeight = desc.ClientHeight;
        m_refreshRate = desc.RefreshRate;
        m_window = desc.Window;

        // Enable Debug Layer
#ifdef _DEBUG
        {
            ComPtr<ID3D12Debug> debugController;
            ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
            debugController->EnableDebugLayer();
        }
#endif

        // Create DXGI Factory
        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

        // Create the Device
        ThrowIfFailed(D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_d3dDevice)));

        // Create the Fence and Descriptor Sizes
        ThrowIfFailed(m_d3dDevice->CreateFence(
            0,
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_fence)));

        m_cbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_samplerDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_dsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

        // Check 4X MSAA Quality Support

        /*D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
        msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        msQualityLevels.SampleCount = 4;
        msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        msQualityLevels.NumQualityLevels = 0;

        ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
            &msQualityLevels,
            sizeof(msQualityLevels)));

        m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
        assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");*/

        CreateCommandObjects();
        CreateSwapChain();
        CreateRtvAndDsvDescriptorHeaps();

        OnResize();
    }

    void Shutdown()
    {
    }

    void OnResize()
    {
        // Flush before changing any resources.
        FlushCommandQueue();

        ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));

        // Release the previous resources we will be recreating.
        for (int i = 0; i < SwapChainBufferCount; ++i)
        {
            m_swapChainBuffer[i].Reset();
        }

        m_depthStencilBuffer.Reset();

        // Resize the swap chain.
        ThrowIfFailed(m_swapChain->ResizeBuffers(
            SwapChainBufferCount,
            m_clientWidth, m_clientHeight,
            m_backBufferFormat,
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

        m_currBackBuffer = 0;

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        for (int i = 0; i < SwapChainBufferCount; ++i)
        {
            // Get the ith buffer in the swap chain.
            ThrowIfFailed(m_swapChain->GetBuffer(
                static_cast<UINT>(i),
                IID_PPV_ARGS(&m_swapChainBuffer[i])));

            // Create an RTV to it.
            m_d3dDevice->CreateRenderTargetView(
                m_swapChainBuffer[i].Get(),
                nullptr,
                rtvHeapHandle);

            // Next entry in heap.
            rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
        }

        // Create the depth/stencil buffer and view.
        D3D12_RESOURCE_DESC depthStencilDesc;
        depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Alignment = 0;
        depthStencilDesc.Width = m_clientWidth;
        depthStencilDesc.Height = m_clientHeight;
        depthStencilDesc.DepthOrArraySize = 1;
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE optClear;
        optClear.Format = m_depthStencilFormat;
        optClear.DepthStencil.Depth = 1.0f;
        optClear.DepthStencil.Stencil = 0;
        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &depthStencilDesc,
            D3D12_RESOURCE_STATE_COMMON,
            &optClear,
            IID_PPV_ARGS(&m_depthStencilBuffer)));

        // Create descriptor to mip level 0 of entire resource using the format of the resource.
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Format = m_depthStencilFormat;
        dsvDesc.Texture2D.MipSlice = 0;
        m_d3dDevice->CreateDepthStencilView(
            m_depthStencilBuffer.Get(),
            &dsvDesc,
            DepthStencilView());

        // Transition the resource from its initial state to be used as a depth buffer.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_depthStencilBuffer.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_DEPTH_WRITE);

        m_commandList->ResourceBarrier(1, &barrier);

        // Execute the resize commands.
        ThrowIfFailed(m_commandList->Close());
        ID3D12CommandList* commandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        FlushCommandQueue();

        // Update the viewport transform to cover the client area.
        m_screenViewport.TopLeftX = 0;
        m_screenViewport.TopLeftY = 0;
        m_screenViewport.Width = static_cast<float>(m_clientWidth);
        m_screenViewport.Height = static_cast<float>(m_clientHeight);
        m_screenViewport.MinDepth = 0.0f;
        m_screenViewport.MaxDepth = 1.0f;

        m_scissorRect = { 0, 0, m_clientWidth, m_clientHeight };
    }

    void CreateCommandObjects()
    {
        D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
        commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        ThrowIfFailed(m_d3dDevice->CreateCommandQueue(
            &commandQueueDesc,
            IID_PPV_ARGS(&m_commandQueue)));

        ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_directCmdListAlloc)));

        ThrowIfFailed(m_d3dDevice->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_directCmdListAlloc.Get(),
            nullptr,
            IID_PPV_ARGS(&m_commandList)));

        m_commandList->Close();
    }

    void CreateSwapChain()
    {
        m_swapChain.Reset();

        DXGI_SWAP_CHAIN_DESC desc;
        desc.BufferDesc.Width = m_clientWidth;
        desc.BufferDesc.Height = m_clientHeight;
        desc.BufferDesc.RefreshRate = m_refreshRate;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = SwapChainBufferCount;
        desc.OutputWindow = m_window;
        desc.Windowed = true;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
            m_commandQueue.Get(),
            &desc,
            &m_swapChain));
    }

    void CreateRtvAndDsvDescriptorHeaps()
    {
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
        rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;

        ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
            &rtvHeapDesc,
            IID_PPV_ARGS(&m_rtvHeap)));

        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NodeMask = 0;

        ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
            &dsvHeapDesc,
            IID_PPV_ARGS(&m_dsvHeap)));
    }

    ID3D12Resource* CurrentBackBuffer()
    {
        return m_swapChainBuffer[m_currBackBuffer].Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(
            m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
            m_currBackBuffer,
            m_rtvDescriptorSize);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()
    {
        return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
    }

    void FlushCommandQueue()
    {
        // Advance the fence value to mark commands up to this fence point.
        ++m_currFence;

        // Add an instruction to the command queue to set a new fence point. Because we
        // are on the GPU timeline, the new fence point won't be set until the GPU finishes
        // processing all the commands prior to this Signal().
        ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currFence));

        // Wait until the GPU has completed commands up to this fence point.
        if (m_fence->GetCompletedValue() < m_currFence)
        {
            HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

            // Fire event when GPU hits current fence.
            ThrowIfFailed(m_fence->SetEventOnCompletion(m_currFence, eventHandle));

            // Wait until the GPU hits current fence event is fired.
            WaitForSingleObject(eventHandle, INFINITE);
            CloseHandle(eventHandle);
        }
    }

    void Draw()
    {
        // Reuse the memory associated with command recording.
        // We can only reset when the associated command lists have finished execution on the GPU.
        ThrowIfFailed(m_directCmdListAlloc->Reset());

        // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
        // Reusing the command list reuses memory.
        ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));

        // Indicate a state transition on the resource usage.
        auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &resourceBarrier);

        // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
        m_commandList->RSSetViewports(1, &m_screenViewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        // Clear the back buffer and depth buffer.
        auto backBufferView = CurrentBackBufferView();

        m_commandList->ClearRenderTargetView(backBufferView, ClearColor, 0, nullptr);
        m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

        // Specify the buffers we are going to render to.
        backBufferView = CurrentBackBufferView();
        auto depthStencilView = DepthStencilView();
        m_commandList->OMSetRenderTargets(1, &backBufferView, true, &depthStencilView);

        // Indicate a state transition on the resource usage.
        resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &resourceBarrier);

        // Done recording commands.
        ThrowIfFailed(m_commandList->Close());

        // Add the command list to the queue for execution.
        ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

        // swap the back and front buffers
        ThrowIfFailed(m_swapChain->Present(0, 0));
        m_currBackBuffer = (m_currBackBuffer + 1) % SwapChainBufferCount;

        // Wait until frame commands are complete.  This waiting is inefficient and is
        // done for simplicity.  Later we will show how to organize our rendering code
        // so we do not have to wait per frame.
        FlushCommandQueue();
    }

    static void LogAdapters()
    {
        UINT adapter = 0;
        IDXGIAdapter* pAdapter = nullptr;

        while (m_dxgiFactory->EnumAdapters(adapter, &pAdapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC desc;
            pAdapter->GetDesc(&desc);

            NGINDX_LOG(L"***Adapter {}\n", desc.Description);

            LogAdapterOutputs(pAdapter);
            SafeRelease(&pAdapter);

            ++adapter;
        }
    }

    static void LogAdapterOutputs(IDXGIAdapter* adapter)
    {
        UINT output = 0;
        IDXGIOutput* pOutput = nullptr;
        while (adapter->EnumOutputs(output, &pOutput) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC desc;
            pOutput->GetDesc(&desc);

            NGINDX_LOG(L"***Output: {}\n", desc.DeviceName);

            LogOutputDisplayModes(pOutput, DXGI_FORMAT_R8G8B8A8_UNORM);
            SafeRelease(&pOutput);

            ++output;
        }
    }

    static void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
    {
        UINT count = 0;
        UINT flags = 0;

        // Call with nullptr to get list count.
        output->GetDisplayModeList(format, flags, &count, nullptr);

        std::vector<DXGI_MODE_DESC> modeList(count);
        output->GetDisplayModeList(format, flags, &count, &modeList[0]);

        for (auto& mode : modeList)
        {
            NGINDX_LOG(L"***Width = {} Height = {} Refresh = {}/{}\n", mode.Width, mode.Height, mode.RefreshRate.Numerator, mode.RefreshRate.Denominator);
        }
    }
}
