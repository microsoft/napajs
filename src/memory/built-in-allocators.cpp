#include <napa-memory.h>
#include <napa-c.h>

/// <summary> C runtime allocator from napa.dll. </summary>
class CrtAllocator: public napa::memory::Allocator {
public:
    /// <summary> Allocate memory of given size. </summary>
    /// <param name="size"> Requested size. </summary>
    /// <returns> Allocated memory. May throw if error happens. </returns>
    void* Allocate(size_t size) override {
        return ::napa_malloc(size);
    }

    /// <summary> Deallocate memory allocated from this allocator. </summary>
    /// <param name="memory"> Pointer to the memory. </summary>
    /// <param name="sizeHint"> Hint of size to delete. 0 if not available from caller. </summary>
    /// <returns> None. May throw if error happens. </returns>
    void CrtAllocator::Deallocate(void* memory, size_t sizeHint) override {
        ::napa_free(memory, sizeHint);
    }

    /// <summary> Get allocator type for better debuggability. </summary>
    const char* CrtAllocator::GetType() const override {
        return "CrtAllocator";
    }

    /// <summary> Tell if another allocator equals to this allocator. </summary>
    bool operator==(const Allocator& other) const override {
        return strcmp(other.GetType(), GetType()) == 0;
    }
};

/// <summary> Allocator that uses napa_allocate and napa_deallocate. </summary>
class DefaultAllocator: public napa::memory::Allocator {
public:
    public:
    /// <summary> Allocate memory of given size. </summary>
    /// <param name="size"> Requested size. </summary>
    /// <returns> Allocated memory. May throw if error happens. </returns>
    void* Allocate(size_t size) override {
        return ::napa_allocate(size);
    }

    /// <summary> Deallocate memory allocated from this allocator. </summary>
    /// <param name="memory"> Pointer to the memory. </summary>
    /// <param name="sizeHint"> Hint of size to delete. 0 if not available from caller. </summary>
    /// <returns> None. May throw if error happens. </returns>
    void Deallocate(void* memory, size_t sizeHint) override {
        return ::napa_deallocate(memory, sizeHint);
    }

    /// <summary> Get allocator type for better debuggability. </summary>
    const char* GetType() const override {
        return "DefaultAllocator";
    }

    /// <summary> Tell if another allocator equals to this allocator. </summary>
    bool operator==(const Allocator& other) const override {
        return strcmp(other.GetType(), GetType()) == 0;
    }
};

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