#include <napa-memory.h>

namespace napa {
    namespace memory {
        namespace {
            CrtAllocator _crtAllocator;
            DefaultAllocator _defaultAllocator;
        } // namespace

        Allocator& GetCrtAllocator() {
            return _crtAllocator;
        }

        Allocator& GetDefaultAllocator() {
            return _defaultAllocator;
        }
    } // namespace memory
} // namespace napa