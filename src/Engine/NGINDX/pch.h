//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

//#ifdef USING_DIRECTX_HEADERS
//#include <directx/dxgiformat.h>
//#include <directx/d3d12.h>
//#include <directx/d3dx12.h>
//#include <dxguids/dxguids.h>
//#else
//#include <d3d12.h>
//
//#include "d3dx12.h"
//#endif

//#include <dxgi1_4.h>

#include <DirectXMath.h>
//#include <DirectXColors.h>

// DirectX Tool Kit headers
#include <GamePad.h>
#include <Keyboard.h>
#include <Mouse.h>

// Standard headers
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <cwchar>
#include <exception>
#include <format>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <tuple>
#include <unordered_map>
#include <vector>

//#ifdef _DEBUG
//#include <dxgidebug.h>
//#endif

// If using the DirectX Tool Kit for DX12, uncomment this line:
//#include "GraphicsMemory.h"

//namespace DX
//{
//    inline void ThrowIfFailed(HRESULT hr)
//    {
//        if (FAILED(hr))
//        {
//            // Set a breakpoint on this line to catch DirectX API errors
//            throw std::exception();
//        }
//    }
//}

//#include <wrl/event.h>


// Project specific
#include "NGINDX/Core/Log.h"
