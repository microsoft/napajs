#pragma once

#ifdef _WIN32
#include <platform/win/dll-impl.h>
#elif defined(__GNUC__)
#include <platform/posix/dll-impl.h>
#else
#error compiler not supported
#endif

#include <string>

namespace napa {
    namespace dll {

        /// <summary> Facade for cross-platform shared library. </summary>
        class SharedLibrary: private SharedLibraryImpl {
        public:
            SharedLibrary(const std::string& dllFileName):
                SharedLibraryImpl(dllFileName) {
            }

            template <typename T>
            T* Import(const std::string& symbolName) {
                auto address = GetSymbolAddress(symbolName);
                return reinterpret_cast<T*>(address);
            }
        };

        /// <summary> Returns the path of the dll including this line of code. </summary>
        static inline std::string ThisLineLocation() {
            return GetSymbolLocation(ThisLineLocation);
        }
    }
}