## 4.3 INITIALIZING DIRECT3D

The following subsections show how to initialize Direct3D for our demo framework. It is a long process, but only needs to be done once. Our process of initializing Direct3D can be broken down into the following steps:

1. Create the `ID3D12Device` using the `D3D12CreateDevice` function.
2. Create an `ID3D12Fence` object and query descriptor sizes.
3. Check 4X MSAA quality level support.
4. Create the command queue, command list allocator, and main command list.
5. Describe and create the swap chain.
6. Create the descriptor heaps the application requires.
7. Resize the back buffer and create a render target view to the back buffer.
8. Create the depth/stencil buffer and its associated depth/stencil view.
9. Set the viewport and scissor rectangles.

### 4.3.1 Create the Device

Initializing Direct3D begins by creating the Direct3D 12 device (`ID3D12Device`). The device represents a display adapter. Usually, the display adapter is a physical piece of 3D hardware (e.g., graphics card); however, a system can also have a software display adapter that emulates 3D hardware functionality (e.g., the WARP adapter). The Direct3D 12 device is used to check feature support, and create all other Direct3D interface objects like resources, views, and command lists. The device can be created with the following function:

```c
HRESULT WINAPI D3D12CreateDevice(
    IUnknown* pAdapter,
    D3D_FEATURE_LEVEL MinimumFeatureLevel,
    REFIID riid, // Expected: ID3D12Device
    void** ppDevice );
```

1. `pAdapter`: Specifies the display adapter we want the created device to represent. Specifying null for this parameter uses the primary display adapter. We always use the primary adapter in the sample programs of this book. §4.1.10 showed how to enumerate all the system’s display adapters.
2. `MinimumFeatureLevel`: The minimum feature level our application requires support for; device creation will fail if the adapter does not support this feature level. In our framework, we specify `D3D_FEATURE_LEVEL_11_0` (i.e., Direct3D 11 feature support).
3. `riid`: The COM ID of the `ID3D12Device` interface we want to create.
4. `ppDevice`: Returns the created device.

### 4.3.2 Create the Fence and Descriptor Sizes

After we have created our device, we can create our fence object for CPU/GPU synchronization. In addition, once we get to working with descriptors, we are going to need to know their size. Descriptor sizes can vary across GPUs so we need to query this information. We cache the descriptor sizes so that it is available when we need it for various descriptor types:

A fence is represented by the `ID3D12Fence` interface and is used to synchronize the GPU and CPU. A fence object can be created with the following method:

```c
HRESULT ID3D12Device::CreateFence(
    UINT64 InitialValue,
    D3D12_FENCE_FLAGS Flags,
    REFIID riid,
    void **ppFence);
```

A fence object maintains a `UINT64` value, which is just an integer to identify a fence point in time. We start at value zero and every time we need to mark a new fence point, we just increment the integer. Now, the following code/comments show how we can use a fence to flush the command queue.

Descriptors have a type, and the type implies how the resource will be used. The types of descriptors we use in this book are:

1. CBV/SRV/UAV descriptors describe constant buffers, shader resources and unordered access view resources.
2. Sampler descriptors describe sampler resources (used in texturing).
3. RTV descriptors describe render target resources.
4. DSV descriptors describe depth/stencil resources.

```c
UINT ID3D12Device::GetDescriptorHandleIncrementSize(
    D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType);
```

### 4.3.3 Check 4X MSAA Quality Support

### 4.3.4 Create Command Queue and Command List

The GPU has a command queue. The CPU submits commands to the queue through the Direct3D API using command lists. It is important to understand that once a set of commands have been submitted to the command queue, they are not immediately executed by the GPU. They sit in the queue until the GPU is ready to process them, as the GPU is likely busy processing previously inserted commands.

Command queue is represented by the `ID3D12CommandQueue` interface, a command allocator is represented by the `ID3D12CommandAllocator` interface, and a command list is represented by the `ID3D12GraphicsCommandList` interface.

In Direct3D 12, the command queue is represented by the `ID3D12CommandQueue` interface. It is created by filling out a `D3D12_COMMAND_QUEUE_DESC` structure describing the queue and then calling `ID3D12Device::CreateCommandQueue`. The way we create our command queue in this book is as follows:

```cpp
Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;

D3D12_COMMAND_QUEUE_DESC queueDesc = {};
queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue))
```

Associated with a command list is a memory backing class called an `ID3D12CommandAllocator`. As commands are recorded to the command list, they
will actually be stored in the associated command allocator. When a command list is executed via `ID3D12CommandQueue::ExecuteCommandLists`, the command queue will reference the commands in the allocator. A command allocator is created from the `ID3D12Device`:

```c
HRESULT ID3D12Device::CreateCommandAllocator(
    D3D12_COMMAND_LIST_TYPE type,
    REFIID riid,
    void **ppCommandAllocator);
```

1. `type`: The type of command lists that can be associated with this allocator. The two common types we use in this book are:
    1. `D3D12_COMMAND_LIST_TYPE_DIRECT`: Stores a list of commands to directly be executed by the GPU (the type of command list we have been describing thus far).
    2. `D3D12_COMMAND_LIST_TYPE_BUNDLE`: Specifies the command list represents a bundle. There is some CPU overhead in building a command list, so Direct3D 12 provides an optimization that allows us to record a sequence of commands into a so-called bundle. After a bundle has been recorded, the driver will preprocess the commands to optimize their execution during rendering. Therefore, bundles should be recorded at initialization time. The use of bundles should be thought of as an optimization to use if profiling shows building particular command lists are taking significant time. The Direct3D 12 drawing API is already very efficient, so you should not need to use bundles often, and you should only use them if you can demonstrate a performance gain by them; that is to say, do not use them by default. We do not use bundles in this book;
see the DirectX 12 documentation for further details.
2. `riid`: The COM ID of the `ID3D12CommandAllocator` interface we want to
create.
3. `ppCommandAllocator`: Outputs a pointer to the created command allocator.

Command lists are also created from the `ID3D12Device`:

```c
HRESULT ID3D12Device::CreateCommandList(
    UINT nodeMask,
    D3D12_COMMAND_LIST_TYPE type,
    ID3D12CommandAllocator *pCommandAllocator,
    ID3D12PipelineState *pInitialState,
    REFIID riid,
    void **ppCommandList);
```

1. `nodeMask`: Set to 0 for single GPU system. Otherwise, the node mask identifies the physical GPU this command list is associated with. In this book we assume single GPU systems.
2. `type`: The type of command list: either `_COMMAND_LIST_TYPE_DIRECT` or `D3D12_COMMAND_LIST_TYPE_BUNDLE`.
3. `pCommandAllocator`: The allocator to be associated with the created command list. The command allocator type must match the command list type.
4. `pInitialState`: Specifies the initial pipeline state of the command list. This can be null for bundles, and in the special case where a command list is executed for initialization purposes and does not contain any draw commands. We discuss `ID3D12PipelineState` in Chapter 6.
5. `riid`: The COM ID of the `ID3D12CommandList` interface we want to create.
6. `ppCommandList`: Outputs a pointer to the created command list.

### 4.3.5 Describe and Create the Swap Chain

The next step in the initialization process is to create the swap chain. This is done by first filling out an instance of the `DXGI_SWAP_CHAIN_DESC` structure, which describes the characteristics of the swap chain we are going to create. This structure is defined as follows:

```c
typedef struct DXGI_SWAP_CHAIN_DESC
{
    DXGI_MODE_DESC BufferDesc;
    DXGI_SAMPLE_DESC SampleDesc;
    DXGI_USAGE BufferUsage;
    UINT BufferCount;
    HWND OutputWindow;
    BOOL Windowed;
    DXGI_SWAP_EFFECT SwapEffect;
    UINT Flags;
} DXGI_SWAP_CHAIN_DESC;
```

The `DXGI_MODE_DESC` type is another structure, defined as:

```c
typedef struct DXGI_MODE_DESC
{
    UINT Width; // Buffer resolution width
    UINT Height; // Buffer resolution height
    DXGI_RATIONAL RefreshRate;
    DXGI_FORMAT Format; // Buffer display format
    DXGI_MODE_SCANLINE_ORDER ScanlineOrdering; //Progressive vs. interlaced
    DXGI_MODE_SCALING Scaling; // How the image is stretched // over the monitor
} DXGI_MODE_DESC;
```

In the following data member descriptions, we only cover the common flags and options that are most important to a beginner at this point. For a description of further flags and options, refer to the SDK documentation.

1. `BufferDesc`: This structure describes the properties of the back buffer we want to create. The main properties we are concerned with are the width and height, and pixel format; see the SDK documentation for further details on the other members.
2. `SampleDesc`: The number of multisamples and quality level; see §4.1.8. For single sampling, specify a sample count of 1 and quality level of 0.
3. `BufferUsage`: Specify `DXGI_USAGE_RENDER_TARGET_OUTPUT` since we are going to be rendering to the back buffer (i.e., use it as a render target).
4. `BufferCount`: The number of buffers to use in the swap chain; specify two for double buffering.
5. `OutputWindow`: A handle to the window we are rendering into.
6. `Windowed`: Specify true to run in windowed mode or false for full-screen mode.
7. `SwapEffect`: Specify `DXGI_SWAP_EFFECT_FLIP_DISCARD`.
8. `Flags`: Optional flags. If you specify `DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH`, then when the application is switching to full-screen mode, it will choose a display mode that best matches the current application window dimensions. If this flag is not specified, then when the application is switching to full-screen mode, it will use the current desktop display mode.

After we have described out swap chain, we can create it with the `IDXGIFactory::CreateSwapChain` method:

```c
HRESULT IDXGIFactory::CreateSwapChain(
    IUnknown *pDevice, // Pointer to ID3D12CommandQueue.
    DXGI_SWAP_CHAIN_DESC *pDesc, // Pointer to swap chain description.
    IDXGISwapChain **ppSwapChain); // Returns created swap chain interface.
```

### 4.3.6 Create the Descriptor Heaps

A descriptor heap is represented by the ID3D12DescriptorHeap interface. A heap is created with the ID3D12Device::CreateDescriptorHeap method. In this chapter’s sample program, we need SwapChainBufferCount many render target views (RTVs) to describe the buffer resources in the swap chain we will render into, and one depth/stencil view (DSV) to describe the depth/stencil buffer resource for depth testing. Therefore, we need a heap for storing SwapChainBufferCount RTVs, and we need a heap for storing one DSV. These heaps are created with the following code:

