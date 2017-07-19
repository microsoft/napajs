// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/memory/allocator.h>
#include <limits>

namespace napa {
namespace stl {
    /// <summary> Allocator is a is a wrapper class for napa::memory::Allocator. It is used to
    /// create an allocator that is compatible with the STL container classes
    /// like map and vector.
    ///
    /// This template declaration is a subset of the declaration in the C++98
    /// spec (ISO/IEC 14882:1998), section 20.4.1.
    /// See specification draft: http://www.open-std.org/jtc1/sc22/open/n2356/.
    /// Also see http://www.codeguru.com/Cpp/Cpp/cpp_mfc/stl/article.php/c4079/.
    /// </summary>
    template <typename T> class Allocator {
    public:
        typedef size_t    size_type;
        typedef ptrdiff_t difference_type;
        typedef T*        pointer;
        typedef const T*  const_pointer;
        typedef T&        reference;
        typedef const T&  const_reference;
        typedef T         value_type;

        template <typename U> struct rebind
        {
            typedef Allocator<U> other;
        };

        /// <summary> Constructor that uses NAPA_DEFAULT_ALLOCATOR. </summary>
        Allocator();

        /// <summary> Constructor that accepts a custom allocator </summary>
        explicit Allocator(napa::memory::Allocator& allocator);

        /// <summary> Copy constructor will be used in assignment of std::shared_ptr in GCC </summary>
        Allocator(const Allocator& other) = default;
        Allocator& operator=(const Allocator&) = default;

        template <typename U> Allocator(const Allocator<U>&) throw();

        /// The following method is in the C++98 specification but was
        /// not implemented. Keeping the declarations here to document
        /// differences from the specification and to help with debugging.
        /// ~Allocator() throw();

        pointer address(reference x) const;
        const_pointer address(const_reference x) const;

        pointer allocate(size_type count, const void* hint = 0);
        void deallocate(pointer p, size_type n);

        size_type max_size() const throw();

        void construct(pointer p, const_reference val);
        void destroy(pointer p);

        bool operator==(const Allocator&) const;
        bool operator!=(const Allocator&) const;

    private:
        template <typename U>
        friend class Allocator;

        napa::memory::Allocator* _allocator;
    };

    template <typename T>
    Allocator<T>::Allocator()
        : _allocator(&napa::memory::GetDefaultAllocator()) {
    }

    template <typename T>
    Allocator<T>::Allocator(napa::memory::Allocator& allocator)
        : _allocator(&allocator) {
    }

    template <typename T>
    template <typename U>
    Allocator<T>::Allocator(const Allocator<U>& rhs) throw()
        : _allocator(rhs._allocator) {
    }

    template <typename T>
    typename Allocator<T>::pointer Allocator<T>::address(typename Allocator<T>::reference x) const {
        return &x;
    }

    template <typename T>
    typename Allocator<T>::const_pointer Allocator<T>::address(typename Allocator<T>::const_reference x) const {
        return &x;
    }

    template <typename T>
    typename Allocator<T>::pointer Allocator<T>::allocate(typename Allocator<T>::size_type count, const void* /*hint*/) {
        return static_cast<Allocator<T>::pointer>(_allocator->Allocate(sizeof(T) * count));
    }

    template <typename T>
    void Allocator<T>::deallocate(typename Allocator<T>::pointer p, typename Allocator<T>::size_type count) {
        _allocator->Deallocate(p, sizeof(T) * count);
    }

    template <typename T>
    typename Allocator<T>::size_type Allocator<T>::max_size() const throw() {
        return std::numeric_limits<Allocator<T>::size_type>::max();
    }

    template <typename T>
    void Allocator<T>::construct(typename Allocator<T>::pointer ptr, typename Allocator<T>::const_reference val) {
        new (ptr) T(val);
    }

#pragma warning(push)
#pragma warning(disable:4100)
    // Warning C4100 says that 'ptr' is unreferenced as a formal parameter.
    // This happens when T is a simple type like int. In this case it seems that
    // the compiler optimizes away the call to ptr->~T(), causing the ptr to
    // seem unreferenced. This seems like a compiler bug.
    template <typename T>
    void Allocator<T>::destroy(typename Allocator<T>::pointer ptr) {
        ptr->~T();
    }
#pragma warning(pop)

    template <typename T>
    bool Allocator<T>::operator==(const Allocator& other) const {
        return *_allocator == *(other._allocator);
    }

    template <typename T>
    bool Allocator<T>::operator!=(const Allocator& other) const {
        return !(*_allocator == *(other._allocator));
    }
}
}
