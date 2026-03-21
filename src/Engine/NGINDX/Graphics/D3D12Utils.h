#pragma once

#include "NGINDX/Core/Utils.h"

namespace NGINDX
{
    class D3DException : public wexception
    {
    private:
        HRESULT m_errorCode{ S_OK };
        std::wstring m_functionName{};
        std::wstring m_filename{};
        int m_lineNumber{ -1 };

    public:
        D3DException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

        std::wstring wwhat() const override;
    };

    template<typename T>
    inline void SafeRelease(T** ppT)
    {
        if (*ppT)
        {
            (*ppT)->Release();
            *ppT = NULL;
        }
    }
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(x) \
{ \
    HRESULT hr__ = (x); \
    std::wstring wfn = NGINDX::AnsiToWString(__FILE__); \
    if(FAILED(hr__)) { throw NGINDX::D3DException(hr__, L#x, wfn, __LINE__); } \
}
#endif
