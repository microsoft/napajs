#pragma once

#include <napa/memory/allocator.h>
#include <sstream>
#include <atomic>

namespace napa {
namespace memory {

    /// <summary> Interface for allocator debugger. </summary>
    class AllocatorDebugger: public Allocator {
    public:
        /// <summary> Get allocator debug information in JSON. 
        /// It's up to implementation to decide the schema of JSON.
        /// </summary>
        virtual std::string GetDebugInfo() const = 0;
    
    protected:
        virtual ~AllocatorDebugger() = default;
    };

    /// <summary> Simple allocator debugger that reports total allocate/deallocate count and total size allocated. </summary>
    class SimpleAllocatorDebugger: public AllocatorDebugger {
    public:
        /// <summary> Constructor </summary>
        /// <param name="allocator"> Actual allocator to allocate/deallocate memory. </summary>
        SimpleAllocatorDebugger(std::shared_ptr<Allocator> allocator)
          : _allocator(std::move(allocator)),
            _allocateCount(0),
            _deallocateCount(0),
            _allocatedSize(0),
            _deallocatedSize(0) {
            std::stringstream stream;
            stream << "SimpleAllocatorDebugger<"
                << _allocator->GetType()
                << ">";
            _typeName = stream.str();
        }

        /// <summary> Allocate memory of given size. </summary>
        /// <param name="size"> Requested size. </summary>
        /// <returns> Allocated memory. May throw if error happens. </returns>
        void* Allocate(size_t size) override {
            _allocateCount++;
            _allocatedSize += size;
            return _allocator->Allocate(size);
        }

        /// <summary> Deallocate memory allocated from this allocator. </summary>
        /// <param name="memory"> Pointer to the memory. </summary>
        /// <param name="sizeHint"> Hint of size to delete. 0 if not available from caller. </summary>
        /// <returns> None. May throw if error happens. </returns>
        void Deallocate(void* memory, size_t sizeHint) override {
            _deallocateCount++;
            _deallocatedSize += sizeHint;
            _allocator->Deallocate(memory, sizeHint);
        }

        /// <summary> Get allocator type for better debuggability. </summary>
        const char* GetType() const override {
            return _typeName.c_str();
        }

        /// <summary> Get allocator debug information in JSON. 
        /// Allocator debugger should own returned buffer.
        /// </summary>
        std::string GetDebugInfo() const override {
            std::stringstream stream;
            stream << "{ "
                << "\"allocate\": " << _allocateCount
                << ", "
                << "\"deallocate\": " << _deallocateCount
                << ", "
                << "\"allocatedSize\": " << _allocatedSize
                << ", "
                << "\"deallocatedSize\": " << _deallocatedSize
                << " }";

            return stream.str();
        }

        /// <summary> Tell if another allocator equals to this allocator. </summary>
        bool operator==(const Allocator& other) const override {
            return strcmp(other.GetType(), GetType()) == 0
                && (_allocator == dynamic_cast<const SimpleAllocatorDebugger*>(&other)->_allocator);
        }

    private:
        /// <summary> No copy, no assignment. </summary>
        SimpleAllocatorDebugger(const SimpleAllocatorDebugger&) = delete;
        SimpleAllocatorDebugger& operator=(const SimpleAllocatorDebugger&) = delete;

        std::shared_ptr<Allocator> _allocator;
        std::string _typeName;

        std::atomic<size_t> _allocateCount;
        std::atomic<size_t> _deallocateCount;
        std::atomic<size_t> _allocatedSize;
        std::atomic<size_t> _deallocatedSize;
    };
}
}