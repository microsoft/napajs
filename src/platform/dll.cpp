// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include <platform/dll.h>
#include <platform/platform.h>

#ifdef SUPPORT_POSIX

#include <dlfcn.h>

#else

#pragma push_macro("NOMINMAX")
#define NOMINMAX
#include <windows.h>
#pragma pop_macro("NOMINMAX")

#endif

#include <stdexcept>

namespace napa {
namespace dll {

    SharedLibrary::SharedLibrary(const std::string& dllFileName) {
#ifdef SUPPORT_POSIX
        _module = dlopen(dllFileName.c_str(), RTLD_LAZY);
#else
        _module = ::LoadLibraryA(dllFileName.c_str());
#endif
        if (_module == nullptr) {
            throw std::runtime_error(dllFileName + " cannot be loaded.");
        }
    }

    SharedLibrary::~SharedLibrary() {
#ifdef SUPPORT_POSIX
        dlclose(_module);
#else
        ::FreeLibrary(static_cast<HMODULE>(_module));
#endif
    }

    void* SharedLibrary::ImportImpl(const std::string& symbolName) {
#ifdef SUPPORT_POSIX
        return reinterpret_cast<void*>(dlsym(_module, symbolName.c_str()));
#else
        return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(_module), symbolName.c_str()));
#endif
    }

    std::string ThisLineLocation() {
        void* symbolAddress = reinterpret_cast<void*>(&ThisLineLocation);

#ifdef SUPPORT_POSIX
        Dl_info info;
        auto result = dladdr(symbolAddress, &info);
        if (result == 0) {
            return "";
        }
        return info.dli_fname;
#else
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
#endif
    }

} // namespace dll
} // namespace napa
