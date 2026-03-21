#include "D3D12Utils.h"

namespace NGINDX
{
    D3DException::D3DException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
        : m_errorCode{ hr }
        , m_functionName{ functionName }
        , m_filename{ filename }
        , m_lineNumber{ lineNumber }
    {
    }

    std::wstring D3DException::wwhat() const
    {
        // Get the string description of the error code.
        _com_error err(m_errorCode);
        std::wstring msg = err.ErrorMessage();

        return m_functionName + L" failed in " + m_filename + L"; line " + std::to_wstring(m_lineNumber) + L"; error: " + msg;
    }
}
