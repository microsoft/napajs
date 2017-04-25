#include <napa-memory.h>

namespace napa {
    namespace memory {
        CrtAllocator crtAllocator;
        DefaultAllocator defaultAllocator;

        Allocator& GetCrtAllocator() {
            return crtAllocator;
        }

        Allocator& GetDefaultAllocator() {
            return defaultAllocator;
        }
    } // namespace memory
} // namespace napa