#include <xos/info.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace xos::info
{
    void log_info( std::ostream& stream )
    {
        HMODULE hModule = nullptr;
        if (GetModuleHandleEx(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCSTR>(&log_info),
                &hModule))
        {
            char path[MAX_PATH];
            DWORD size = GetModuleFileName(hModule, path, MAX_PATH);
            if (size > 0)
            {
                stream << "XOS lib loaded from: " << path << std::endl;
                return;
            }
        }
        throw std::runtime_error("Failed to retrieve library handle (GetModuleHandleEx failed)!");
    }

}
