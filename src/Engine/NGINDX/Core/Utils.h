#pragma once

#include "NGINDX/Core/Log.h"

namespace NGINDX
{
    class wexception : public std::exception
    {
    public:
        virtual std::wstring wwhat() const = 0;
    };

    std::wstring AnsiToWString(const std::string& str);
}
