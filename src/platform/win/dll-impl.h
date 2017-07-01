#pragma once

#define NOMINMAX
#include <windows.h>
#undef NOMINMAX

#include <string>

namespace napa {
namespace dll {

    /// <summary> Windows implementation for SharedLibrary. </summary>
    class SharedLibraryImpl {
    public:
        SharedLibraryImpl(const std::string& dllName) {
            // TODO: support unicode later.
            _module = ::LoadLibraryA(dllName.c_str());
            if (_module == NULL) {
                throw std::runtime_error(dllName + " cannot be loaded.");
            }
        }

        virtual ~SharedLibraryImpl() {
            ::FreeLibrary(_module);
        }

        void* GetSymbolAddress(const std::string& symbolName) {
            return ::GetProcAddress(_module, symbolName.c_str());
        }

    private:
        HMODULE _module;
    };

    /// <summary> Returns the path of symbol address. </summary>
    inline std::string GetSymbolLocation(void* symbolAddress) {
        MEMORY_BASIC_INFORMATION mbi;
        auto size = ::VirtualQuery(symbolAddress, &mbi, sizeof(mbi));
        if (size == 0) {
            return "";
        }

        static constexpr size_t DEFAULT_PATH_SIZE = 1024;
        char path[DEFAULT_PATH_SIZE];
        auto len = ::GetModuleFileNameA(reinterpret_cast<HMODULE>(mbi.AllocationBase), path, sizeof(path));
        if (len == 0) {
            return "";
        }
        return path;
    }
}
}