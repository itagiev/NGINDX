#pragma once

namespace NGINDX
{
    template<typename... Args>
    void Log(const std::wformat_string<Args...> fmt, Args&&... args);
}

#ifdef _DEBUG
#define NGINDX_LOG(...) ::NGINDX::Log(__VA_ARGS__)
#else
#define NGINDX_LOG(...) ((void)0)
#endif
