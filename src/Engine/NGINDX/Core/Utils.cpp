#include "Utils.h"

namespace NGINDX
{
    std::wstring AnsiToWString(const std::string& str)
    {
        // Calculate the needed buffer size (including null terminator)
        int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
        if (size_needed == 0) {
            // Handle error
            return L"";
        }

        // Allocate buffer for the wide string
        std::wstring wstr(size_needed, 0);

        // Perform the conversion
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wstr[0], size_needed);

        // Remove the null terminator from the std::wstring object
        wstr.resize(size_needed - 1);

        return wstr;
    }
}
