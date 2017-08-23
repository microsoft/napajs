// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.
#pragma once

#include <napa/assert.h>
#include <platform/platform.h>

#ifdef SUPPORT_POSIX

#include <pthread.h>

#else

#include <windows.h>

#endif

namespace napa {
namespace tls {

    /// <summary> Cross platform implementation for Tls. </summary>
    /// <remarks> Use thread_local once major clang versions supports it. </remarks>
    template <typename T>
    class ThreadLocal {
    public:
        ThreadLocal() {
#ifdef SUPPORT_POSIX
            auto result = pthread_key_create(&_key, nullptr);
            NAPA_ASSERT(result == 0, "Failed to create key for TLS");
#else
            _key = TlsAlloc();
            NAPA_ASSERT(_key != TLS_OUT_OF_INDEXES, "Failed to create key for TLS");
#endif
        }

        ~ThreadLocal() {
            auto object = TlsGet();
            if (object != nullptr) {
                delete object;
            }

#ifdef SUPPORT_POSIX
            auto result = pthread_key_delete(_key);
            NAPA_ASSERT(result == 0, "Failed to delete key for TLS");
#else
            auto result = TlsFree(_key);
            NAPA_ASSERT(result == TRUE, "Failed to delete key ");
#endif
        }

        /// <summary> Per-thread install object of T.  </summary>
        template <typename... Args>
        void Install(Args&&... args) {
            auto object = new T(std::forward<Args>(args)...);
            TlsSet(object);
        }

        T& operator*() {
            return *TlsGet();
        }

        T* operator->() {
            return TlsGet();
        }

        const T& operator*() const {
            return *TlsGet();
        }

        const T* operator->() const {
            return TlsGet();
        }

        /// <summary> Reset current Tls slot with another pointer. </summary>
        void Reset(T* object) {
            T* old = TlsGet();
            if (old != nullptr) {
                delete old;
            }
            TlsSet(object);
        }

    private:
        /// <summary> Get const pointer of T in current Tls slot.  </summary>
        T* TlsGet() const {
#ifdef SUPPORT_POSIX
            return reinterpret_cast<T*>(pthread_getspecific(_key));
#else
            return reinterpret_cast<T*>(TlsGetValue(_key));
#endif
        }

        /// <summary> Set value for current Tls slot. </summary>
        void TlsSet(const void* object) {
#ifdef SUPPORT_POSIX
            auto result = pthread_setspecific(_key, object);
            NAPA_ASSERT(result == 0, "Failed to set value in Tls");
#else
            auto result = TlsSetValue(_key, (LPVOID)object);
            NAPA_ASSERT(result == TRUE, "Failed to set value in Tls");
#endif
        }

#ifdef SUPPORT_POSIX
        pthread_key_t _key;
#else
        DWORD _key;
#endif
    };
} // namespace tls
} // namespace napa