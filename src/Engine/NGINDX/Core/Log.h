#pragma once

namespace NGINDX
{
    template<typename... Args>
    inline void Log(const std::wformat_string<Args...> fmt, Args&&... args)
    {
        OutputDebugStringW(std::format(fmt, std::forward<Args>(args)...).c_str());
    }
}

#ifdef _DEBUG
#ifndef NGINDX_LOG
#define NGINDX_LOG(...) ::NGINDX::Log(__VA_ARGS__)
#endif
#else
#ifndef NGINDX_LOG
#define NGINDX_LOG(...) ((void)0)
#endif
#endif
