// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/capi.h>
#include <napa/memory/allocator.h>
#include <napa/stl/allocator.h>
#include <memory>
#include <type_traits>

namespace napa {
namespace memory {
    /// <summary> Deleter using Napa's default allocator. </summary>
    template <typename T>
    void DefaultDeleter(T* object) {
        static_assert(std::is_destructible<T>::value, "Not destructible.");
        object->~T();
        ::napa_deallocate(object, sizeof(T));
    }

    /// <summary> std::unique_ptr using Napa default allocator. </summary>
    template <typename T>
    using UniquePtr = std::unique_ptr<T, void (*)(T*)>;

    /// <summary> std::make_unique using Napa default allocator. </summary>
    template <typename T, typename... Args>
    UniquePtr<T> MakeUnique(Args&&... args) {
        void* memory = ::napa_allocate(sizeof(T));
        T* t = new (memory) T(std::forward<Args>(args)...);
        return UniquePtr<T>(t, DefaultDeleter<T>);
    }

    /// <summary> std::allocate_shared using napa::memory::Allocator. </summary>
    template <typename T, typename Alloc, typename... Args>
    std::shared_ptr<T> AllocateShared(Alloc& allocator, Args&&... args) {
        return std::allocate_shared<T>(
            napa::stl::Allocator<T>(allocator), 
            std::forward<Args>(args)...);
    }

    /// <summary> std::make_shared using Napa default allocator. </summary>
    template <typename T, typename... Args>
    std::shared_ptr<T> MakeShared(Args&&... args) {
        return AllocateShared<T>(
            GetDefaultAllocator(), 
            std::forward<Args>(args)...);
    }
}
}