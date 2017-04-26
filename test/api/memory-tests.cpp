#include "catch.hpp"

#include "napa-initialization-guard.h"

#include <napa-memory.h>
#include <napa/memory/allocator-debugger.h>
#include <napa/stl/allocator.h>
#include <napa/stl/string.h>
#include <napa/stl/vector.h>
#include <napa/stl/map.h>

using namespace napa::memory;

// Make sure Napa is initialized exactly once.
static NapaInitializationGuard _guard;

namespace custom_allocator {
    char buffer[1024];
    size_t allocated = 0;
    size_t countMalloc = 0;
    size_t countFree = 0;

    void* malloc(size_t size) {
        void* pointer =  buffer + allocated;
        allocated += size;
        countMalloc++;
        return pointer;
    }

    void free(void* pointer, size_t sizeHint) {
        countFree++;
        allocated -= sizeHint;

        // Do nothing but check memory are allocated from buffer.
        if (pointer < buffer && pointer >= buffer + sizeof(buffer)) {
            throw std::exception("bad memory in free");
        }
    }

    void reset() {
        allocated = 0;
        countMalloc = 0;
        countFree = 0;
    }
}

TEST_CASE("Napa memory C api tests", "[memory-capi]") {
    SECTION("C runtime malloc/free from napa.dll") {
        void* pointer = NAPA_MALLOC(4);
        REQUIRE(pointer != nullptr);

        NAPA_FREE(pointer, 4);
    }
    
    SECTION("Set default allocator and use it") {
        NAPA_SET_DEFAULT_ALLOCATOR(custom_allocator::malloc, custom_allocator::free);

        size_t size = 5;
        void* pointer = NAPA_ALLOCATE(size);
        REQUIRE(pointer == custom_allocator::buffer);
        REQUIRE(custom_allocator::allocated == size);
        REQUIRE(custom_allocator::countMalloc == 1);

        NAPA_DEALLOCATE(pointer, size);
        REQUIRE(custom_allocator::countFree == 1);
        REQUIRE(custom_allocator::allocated == 0);

        NAPA_RESET_DEFAULT_ALLOCATOR();
        custom_allocator::reset();
    }
}

TEST_CASE("Napa memory allocator tests", "[memory-allocators]") {
    SECTION("Test CrtAllocator.") {
        auto& allocator = NAPA_CRT_ALLOCATOR;
        REQUIRE(std::string(allocator.GetType()) == "CrtAllocator");

        auto pointer = allocator.Allocate(4);
        REQUIRE(pointer != nullptr);

        allocator.Deallocate(pointer, 4);
    }

    SECTION("Test GetDefaultAllocator.") {
        NAPA_SET_DEFAULT_ALLOCATOR(custom_allocator::malloc, custom_allocator::free);

        auto& allocator = NAPA_DEFAULT_ALLOCATOR;
        REQUIRE(std::string(allocator.GetType()) == "DefaultAllocator");

        size_t size = 5;
        auto pointer = allocator.Allocate(size);
        REQUIRE(pointer == custom_allocator::buffer);
        REQUIRE(custom_allocator::allocated == size);
        REQUIRE(custom_allocator::countMalloc == 1);

        allocator.Deallocate(pointer, size);
        REQUIRE(custom_allocator::countFree == 1);
        REQUIRE(custom_allocator::allocated == 0);

        NAPA_RESET_DEFAULT_ALLOCATOR();
        custom_allocator::reset();
    }

    SECTION("Test CreateSimpleAllocatorDebugger") {
        NAPA_SET_DEFAULT_ALLOCATOR(custom_allocator::malloc, custom_allocator::free);

        SimpleAllocatorDebugger debugger(std::make_shared<DefaultAllocator>());
        REQUIRE(std::string(debugger.GetType()) == "SimpleAllocatorDebugger<DefaultAllocator>");

        size_t size = 5;
        auto pointer = debugger.Allocate(size);
        REQUIRE(pointer == custom_allocator::buffer);
        REQUIRE(custom_allocator::allocated == size);
        REQUIRE(custom_allocator::countMalloc == 1);

        debugger.Deallocate(pointer, size);
        REQUIRE(custom_allocator::countFree == 1);

        REQUIRE(debugger.GetDebugInfo() == "{ \"allocate\": 1, \"deallocate\": 1, \"allocatedSize\": 5, \"deallocatedSize\": 5 }");
        NAPA_RESET_DEFAULT_ALLOCATOR();
        custom_allocator::reset();
    }
}

struct Foo {
    napa::stl::String _str;
    napa::stl::Vector<int> _vector;
    napa::stl::Map<size_t, int> _map;
    Foo(const char* str, size_t size, int elements[])
        : _str(str) {
        for (size_t i = 0; i < size; ++i) {
            _vector.push_back(elements[i]);
            _map.insert(std::make_pair(i, elements[i]));
        }
    }
};

TEST_CASE("Memory helpers", "[memory-helpers]") {
    NAPA_SET_DEFAULT_ALLOCATOR(custom_allocator::malloc, custom_allocator::free);

    SECTION("NAPA_MAKE_UNIQUE") {
        constexpr size_t count = 2;
        int elems[count] = {1, 2};
        {
            auto pointer = NAPA_MAKE_UNIQUE<Foo>("hello world", count, elems);
            REQUIRE(pointer != nullptr);

            REQUIRE(pointer->_str == "hello world");
            REQUIRE(pointer->_vector.size() == 2);
            REQUIRE(pointer->_map.size() == 2);
        }
        REQUIRE(custom_allocator::allocated == 0);
        custom_allocator::reset();
    }

    SECTION("NAPA_MAKE_SHARED") {
        constexpr size_t count = 2;
        int elems[count] = { 1, 2 };
        {
            auto pointer = NAPA_MAKE_SHARED<Foo>("hello world", count, elems);
            REQUIRE(pointer != nullptr);

            REQUIRE(pointer->_str == "hello world");
            REQUIRE(pointer->_vector.size() == 2);
            REQUIRE(pointer->_map.size() == 2);
        }
        REQUIRE(custom_allocator::allocated == 0);
        custom_allocator::reset();
    }

    NAPA_RESET_DEFAULT_ALLOCATOR();
}