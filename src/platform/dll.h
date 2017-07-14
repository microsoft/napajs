#pragma once

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
            return reinterpret_cast<T*>(ImportImpl(symbolName));
        }

    private:
        void* ImportImpl(const std::string& symbolName);

        void* _module;
    };

    /// <summary> Returns the path of the dll including this line of code. </summary>
    std::string ThisLineLocation();
}
}