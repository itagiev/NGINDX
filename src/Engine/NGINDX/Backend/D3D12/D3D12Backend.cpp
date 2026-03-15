#include "NGINDX/Backend/D3D12/D3D12Backend.h"

namespace NGINDX::D3D12
{
    // Usings
    using Microsoft::WRL::ComPtr;
    using namespace DirectX;

    // Members
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;

    void Initialize()
    {
#ifdef _DEBUG
        ComPtr<ID3D12Debug> debugController;
        ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
#endif


        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

        // Try to create hardware device.
        ThrowIfFailed(D3D12CreateDevice(
            nullptr,
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&d3dDevice)));



//        // Fallback to WARP device.
//        if (FAILED(hardwareResult))
//        {
//            ComPtr<IDXGIAdapter> pWarpAdapter;
//            ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));
//
//            ThrowIfFailed(D3D12CreateDevice(
//                pWarpAdapter.Get(),
//                D3D_FEATURE_LEVEL_11_0,
//                IID_PPV_ARGS(&md3dDevice)));
//        }
//
//        ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
//            IID_PPV_ARGS(&mFence)));
//
//        mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//        mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
//        mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//        // Check 4X MSAA quality support for our back buffer format.
//        // All Direct3D 11 capable devices support 4X MSAA for all render 
//        // target formats, so we only need to check quality support.
//
//        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
//        msQualityLevels.Format = mBackBufferFormat;
//        msQualityLevels.SampleCount = 4;
//        msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
//        msQualityLevels.NumQualityLevels = 0;
//        ThrowIfFailed(md3dDevice->CheckFeatureSupport(
//            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
//            &msQualityLevels,
//            sizeof(msQualityLevels)));
//
//        m4xMsaaQuality = msQualityLevels.NumQualityLevels;
//        assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
//
//#ifdef _DEBUG
//        LogAdapters();
//#endif
//
//        CreateCommandObjects();
//        CreateSwapChain();
//        CreateRtvAndDsvDescriptorHeaps();
//
//        return true;
    }

    void Shutdown()
    {
    }
}
