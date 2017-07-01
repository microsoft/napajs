#pragma once

#include <dlfcn.h>

#include <string>

namespace napa {
namespace dll {

    /// <summary> POSIX implementation for SharedLibrary. </summary>
    class SharedLibraryImpl {
    public:
        SharedLibraryImpl(const std::string& dllName) {
            _module = dlopen(dllName.c_str(), RTLD_LAZY);
            if (_module == nullptr) {
                throw std::runtime_error(dllName + " cannot be loaded.");
            }
        }

        virtual ~SharedLibraryImpl() {
            dlclose(_module);
        }

        void* GetSymbolAddress(const std::string& symbolName) {
            return dlsym(_module, symbolName.c_str());
        }

    private:
        void* _module;
    };

    /// <summary> Returns the path of symbol address. </summary>
    inline std::string GetSymbolLocation(void* symbolAddress) {
        Dl_info info;
        auto result = dladdr(symbolAddress, &info);
        if (result == 0) {
            return "";
        }
        return info.dli_fname;
    }
}
}