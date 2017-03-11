#pragma once

#include <cstdint>

namespace napa {
namespace module {
namespace tls {

    /// <summary> Allocate a thread local storage index. </summary>
    uint32_t Alloc();

    /// <summary> Release a thread local storage index. </summary>
    /// <param name="index"> The index allocated by TlsAlloc(). </param>
    /// <returns> It returns true when the function succeeds. </returns>
    bool Free(uint32_t index);

    /// <summary> Retrieves the value in the calling thread's thread local storage slot for the given index. </summary>
    /// <param name="index"> The index allocated by TlsAlloc(). </param>
    /// <returns> The value stored at the calling thread's thread local storage slot. </returns>
    void* GetValue(uint32_t index);

    /// <summary> Store a value in the calling thread's thread local storage slot for the given index. </summary>
    /// <param name="index"> The index allocated by TlsAlloc(). </param>
    /// <param name="value"> The value to be stored. </param>
    /// <returns> It returns true when the function succeeds. </returns>
    bool SetValue(uint32_t index, void* value);

}   // End of namespce tls.
}   // End of namespce module.
}   // End of namespce napa.