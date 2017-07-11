#pragma once

#if defined(__POSIX__)

#include <dlfcn.h>

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

#define NOMINMAX
#include <windows.h>
#undef NOMINMAX

#else

#error platform not supported

#endif

#include <string>

namespace napa {
    namespace dll {

        /// <summary> Cross-platform shared library access. </summary>
        class SharedLibrary {
        public:
            SharedLibrary(const std::string& dllFileName);
            virtual ~SharedLibrary();

            template <typename T>
            T* Import(const std::string& symbolName) {
#ifdef __POSIX__
                return reinterpret_cast<T*>(dlsym(_module, symbolName.c_str()));
#else
                return reinterpret_cast<T*>(::GetProcAddress(_module, symbolName.c_str()));
#endif
            }
        private:
#ifdef __POSIX__
            void* _module;
#else
            HMODULE _module;
#endif
        };

        inline SharedLibrary::SharedLibrary(const std::string& dllFileName) {
#ifdef __POSIX__
            _module = dlopen(dllFileName.c_str(), RTLD_LAZY);
#else
            _module = ::LoadLibraryA(dllFileName.c_str());
#endif
            if (_module == nullptr) {
                throw std::runtime_error(dllFileName + " cannot be loaded.");
            }
        }

        inline SharedLibrary::~SharedLibrary() {
#ifdef __POSIX__
            dlclose(_module);
#else
            ::FreeLibrary(_module);
#endif
        }

        /// <summary> Returns the path of the dll including this line of code. </summary>
        static inline std::string ThisLineLocation() {
            void* symbolAddress = reinterpret_cast<void*>(ThisLineLocation);

#ifdef __POSIX__
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
    }
}