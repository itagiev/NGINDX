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

A descriptor heap is represented by the `ID3D12DescriptorHeap` interface. A heap is created with the `ID3D12Device::CreateDescriptorHeap` method. In this chapter’s sample program, we need `SwapChainBufferCount` many render target views (RTVs) to describe the buffer resources in the swap chain we will render into, and one depth/stencil view (DSV) to describe the depth/stencil buffer resource for depth testing. Therefore, we need a heap for storing `SwapChainBufferCount` RTVs, and we need a heap for storing one DSV. These heaps are created with the following code:

### 4.3.7 Create the Render Target View

We must create a resource view (descriptor) to the resource and bind the view to the pipeline stage. In particular, in order to bind the back buffer to the output merger stage of the pipeline (so Direct3D can render onto it), we need to create a render target view to the back buffer. The first step is to get the buffer resources which are stored in the swap chain:

```c
HRESULT IDXGISwapChain::GetBuffer(
    UINT Buffer,
    REFIID riid,
    void **ppSurface);
```

1. `Buffer`: An index identifying the particular back buffer we want to get (in case there is more than one).
2. `riid`: The COM ID of the `ID3D12Resource` interface we want to obtain a pointer to.
3. `ppSurface`: Returns a pointer to an `ID3D12Resource` that represents the back buffer.

The call to `IDXGISwapChain::GetBuffer` increases the COM reference count to the back buffer, so we must release it when we are finished with it. This is done automatically if using a ComPtr. To create the render target view, we use the `ID3D12Device::CreateRenderTargetView` method:

```c
void ID3D12Device::CreateRenderTargetView(
    ID3D12Resource *pResource,
    const D3D12_RENDER_TARGET_VIEW_DESC *pDesc,
    D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
```

1. `pResource`: Specifies the resource that will be used as the render target, which, in the example above, is the back buffer (i.e., we are creating a render target view to the back buffer).
2. `pDesc`: A pointer to a `D3D12_RENDER_TARGET_VIEW_DESC`. Among other things, this structure describes the data type (format) of the elements in the resource. If the resource was created with a typed format (i.e., not typeless), then this parameter can be null, which indicates to create a view to the first mipmap level of this resource (the back buffer only has one mipmap level) with the format the resource was created with. (Mipmaps are discussed in Chapter 9.) Because we specified the type of our back buffer, we specify null for this parameter.
3. `DestDescriptor`: Handle to the descriptor that will store the created render target view.

### 4.3.8 Create the Depth/Stencil Buffer and View

We now need to create the depth/stencil buffer. As described in §4.1.5, the depth buffer is just a 2D texture that stores the depth information of the nearest visible objects (and stencil information if using stenciling). A texture is a kind of GPU resource, so we create one by filling out a `D3D12_RESOURCE_DESC` structure describing the texture resource, and then calling the `ID3D12Device::CreateCommittedResource` method. The `D3D12_RESOURCE_DESC` structure is defined as follows:

```c
typedef struct D3D12_RESOURCE_DESC
{
    D3D12_RESOURCE_DIMENSION Dimension;
    UINT64 Alignment;
    UINT64 Width;
    UINT Height;
    UINT16 DepthOrArraySize;
    UINT16 MipLevels;
    DXGI_FORMAT Format;
    DXGI_SAMPLE_DESC SampleDesc;
    D3D12_TEXTURE_LAYOUT Layout;
    D3D12_RESOURCE_MISC_FLAG MiscFlags;
} D3D12_RESOURCE_DESC;
```

1. `Dimension`: The dimension of the resource, which is one of the following enumerated types:

    ```c
    enum D3D12_RESOURCE_DIMENSION
    {
        D3D12_RESOURCE_DIMENSION_UNKNOWN = 0,
        D3D12_RESOURCE_DIMENSION_BUFFER = 1,
        D3D12_RESOURCE_DIMENSION_TEXTURE1D = 2,
        D3D12_RESOURCE_DIMENSION_TEXTURE2D = 3,
        D3D12_RESOURCE_DIMENSION_TEXTURE3D = 4
    } D3D12_RESOURCE_DIMENSION;
    ```

2. `Width`: The width of the texture in texels. For buffer resources, this is the number of bytes in the buffer.
3. `Height`: The height of the texture in texels.
4. `DepthOrArraySize`: The depth of the texture in texels, or the texture array size (for 1D and 2D textures). Note that you cannot have a texture array of 3D textures.
5. `MipLevels`: The number of mipmap levels. Mipmaps are covered in Chapter 9 on texturing. For creating the depth/stencil buffer, our texture only needs one mipmap level.
6. `Format`: A member of the `DXGI_FORMAT` enumerated type specifying the format of the texels. For a depth/stencil buffer, this needs to be one of the formats shown in §4.1.5.
7. `SampleDesc`: The number of multisamples and quality level; see §4.1.7 and §4.1.8. Recall that 4X MSAA uses a back buffer and depth buffer 4X bigger than the screen resolution, in order to store color and depth/stencil information per subpixel. Therefore, the multisampling settings used for the depth/stencil buffer must match the settings used for the render target.
8. `Layout`: A member of the `D3D12_TEXTURE_LAYOUT` enumerated type that specifies the texture layout. For now, we do not have to worry about the layout and can specify `D3D12_TEXTURE_LAYOUT_UNKNOWN`.
9. `MiscFlags`: Miscellaneous resource flags. For a depth/stencil buffer resource, specify `D3D12_RESOURCE_MISC_DEPTH_STENCIL`.

GPU resources live in heaps, which are essentially blocks of GPU memory with certain properties. The ID3D12Device::CreateCommittedResource method creates and commits a resource to a particular heap with the properties we specify.

```c
HRESULT ID3D12Device::CreateCommittedResource(
    const D3D12_HEAP_PROPERTIES *pHeapProperties,
    D3D12_HEAP_MISC_FLAG HeapMiscFlags,
    const D3D12_RESOURCE_DESC *pResourceDesc,
    D3D12_RESOURCE_USAGE InitialResourceState,
    const D3D12_CLEAR_VALUE *pOptimizedClearValue,
    REFIID riidResource,
void **ppvResource);

typedef struct D3D12_HEAP_PROPERTIES {
    D3D12_HEAP_TYPE Type;
    D3D12_CPU_PAGE_PROPERTIES CPUPageProperties;
    D3D12_MEMORY_POOL MemoryPoolPreference;
    UINT CreationNodeMask;
    UINT VisibleNodeMask;
} D3D12_HEAP_PROPERTIES;
```

1. `pHeapProperties`: The properties of the heap we want to commit the resource to. Some of these properties are for advanced usage. For now, the main property we need to worry about is the `D3D12_HEAP_TYPE`, which can be one of the following members of the `D3D12_HEAP_PROPERTIES` enumerated type:
    1. `D3D12_HEAP_TYPE_DEFAULT`: Default heap. This is where we commit resources that will be solely accessed by the GPU. Take the depth/stencil buffer as an example: The GPU reads and writes to the depth/stencil buffer. The CPU never needs access to it, so the depth/stencil buffer would be placed in the default heap.
    2. `D3D12_HEAP_TYPE_UPLOAD`: Upload heap. This is where we commit resources where we need to upload data from the CPU to the GPU resource.
    3. `D3D12_HEAP_TYPE_READBACK`: Read-back heap. This is where we commit resources that need to be read by the CPU.
    4. `D3D12_HEAP_TYPE_CUSTOM`: For advanced usage scenarios—see the MSDN documentation for more information.
2. `HeapMiscFlags`: Additional flags about the heap we want to commit the resource to. This will usually be `D3D12_HEAP_MISC_NONE`.
3. `pResourceDesc`: Pointer to a `D3D12_RESOURCE_DESC` instance describing the resource we want to create.
4. `InitialResourceState`: Recall from §4.2.3 that resources have a current usage state. Use this parameter to set the initial state of the resource when it is created. For the depth/stencil buffer, the initial state will be `D3D12_RESOURCE_USAGE_INITIAL`, and then we will want to transition it to the `D3D12_RESOURCE_USAGE_DEPTH` so it can be bound to the pipeline as a depth/stencil buffer.
5. `pOptimizedClearValue`: Pointer to a `D3D12_CLEAR_VALUE` object that describes an optimized value for clearing resources. Clear calls that match the optimized clear value can potentially be faster than clear calls that do not match the optimized clear value. Null can also be specified for this value to not specify an optimized clear value.

    ```c
    struct D3D12_CLEAR_VALUE
    {
        DXGI_FORMAT Format;
        union
        {
            FLOAT Color[ 4 ];
            D3D12_DEPTH_STENCIL_VALUE DepthStencil;
        };
    } D3D12_CLEAR_VALUE;
    ```

6. `riidResource`: The COM ID of the `ID3D12Resource` interface we want to obtain a pointer to.
7. `ppvResource`: Returns pointer to an `ID3D12Resource` that represents the newly created resource.

### 4.3.9 Set the Viewport

The subrectangle of the back buffer we draw into is called the viewport and it is described by the following structure:

```c
typedef struct D3D12_VIEWPORT {
    FLOAT TopLeftX;
    FLOAT TopLeftY;
    FLOAT Width;
    FLOAT Height;
    FLOAT MinDepth;
    FLOAT MaxDepth;
} D3D12_VIEWPORT;
```

The first four data members define the viewport rectangle relative to the back buffer (observe that we can specify fractional pixel coordinates because the data members are of type float). In Direct3D, depth values are stored in the depth buffer in a normalized range of 0 to 1. The `MinDepth` and `MaxDepth` members are used to transform the depth interval [0, 1] to the depth interval [`MinDepth`, `MaxDepth`]. Being able to transform the depth range can be used to achieve certain effects; for example, you could set `MinDepth`=0 and `MaxDepth`=0, so that all objects drawn with this viewport will have depth values of 0 and appear in front of all other objects in the scene. However, usually `MinDepth` is set to 0 and `MaxDepth` is set to 1 so that the depth values are not modified.

### 4.3.10 Set the Scissor Rectangles

We can define a scissor rectangle relative to the back buffer such that pixels outside this rectangle are culled (i.e., not rasterized to the back buffer). This can be used for optimizations. For example, if we know an area of the screen will contain a rectangular UI element on top of everything, we do not need to process the pixels of the 3D world that the UI element will obscure.

A scissor rectangle is defined by a `D3D12_RECT` structure which is typedefed to the following structure:

```c
typedef struct tagRECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;
```

We set the scissor rectangle with Direct3D with the `ID3D12CommandList::RSSetScissorRects` method. The following example creates and sets a scissor rectangle that covers the upper-left quadrant of the back buffer:

```c
mScissorRect = { 0, 0, mClientWidth/2, mClientHeight/2 };
mCommandList->RSSetScissorRects(1, &mScissorRect);
```

Similar to RSSetViewports, the first parameter is the number of scissor rectangles to bind (using more than one is for advanced effects), and the second parameter is a pointer to an array of rectangles.
