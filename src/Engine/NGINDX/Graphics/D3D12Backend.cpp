#include "D3D12Backend.h"

namespace NGINDX::D3D12
{
    // Usings
    using Microsoft::WRL::ComPtr;


    // Members
    ComPtr<IDXGIFactory4> m_dxgiFactory;
    ComPtr<ID3D12Device> m_d3dDevice;                       // Represents a display adapter
    ComPtr<ID3D12Fence> m_fence;                            // Synchronize the GPU and CPU

    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12CommandAllocator> m_directCmdListAlloc;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;

    ComPtr<IDXGISwapChain> m_swapChain;

    UINT m_cbvSrvUavDescriptorSize;     // CBV/SRV/UAV descriptors describe constant buffers, shader resources and unordered access view resources
    UINT m_samplerDescriptorSize;       // Sampler descriptors describe sampler resources (used in texturing)
    UINT m_rtvDescriptorSize;           // RTV descriptors describe render target resources
    UINT m_dsvDescriptorSize;           // DSV descriptors describe depth/stencil resources

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;     // RTV descriptors describe render target resources
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;     // DSV descriptors describe depth/stencil resources

    //bool m_4xMsaaState = true;      // 4X MSAA enabled
    //UINT m_4xMsaaQuality = 0;       // quality level of 4X MSAA

    // SHOULD BE SET FROM APPLICATION
    D3D_DRIVER_TYPE m_d3dDriverType;
    DXGI_FORMAT m_backBufferFormat;
    DXGI_FORMAT m_depthStencilFormat;
    int m_clientWidth;
    int m_clientHeight;
    DXGI_RATIONAL m_refreshRate;
    HWND m_window;


    // Forward declaration
    static void LogAdapters();
    static void LogAdapterOutputs(IDXGIAdapter* adapter);
    static void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

    void CreateCommandObjects();
    void CreateSwapChain();
    void CreateRtvAndDsvDescriptorHeaps();
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView();


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
    }

    void Shutdown()
    {
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
        desc.BufferCount = g_swapChainBufferCount;
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
        rtvHeapDesc.NumDescriptors = g_swapChainBufferCount;
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

    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()
    {
        // TODO: add d3dx12;
        return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
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
