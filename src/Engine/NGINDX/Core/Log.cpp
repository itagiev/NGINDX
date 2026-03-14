#include "Log.h"

namespace NGINDX
{
    template<typename... Args>
    void Log(const std::wformat_string<Args...> fmt, Args&&... args)
    {
        OutputDebugStringW(std::format(fmt, std::forward<Args>(args)...).c_str());
    }
}
