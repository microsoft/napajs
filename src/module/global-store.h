#pragma once

#include <napa/exports.h>

namespace napa {
namespace module {
namespace global_store {

    /// <summary> Retrieve a value from the global store. </summary>
    /// <param name="key"> A key identifying the desired value. </param>
    /// <returns> The value if exists, nullptr otherwise. </returns>
    void* GetValue(const char* key);

    /// <summary> Set a value (opaque pointer) into the  global store. </summary>
    /// <param name="key"> A key identifying the desired value. </param>
    /// <param name="value"> The value to store (An opaque pointer). </param>
    /// <returns> True if value was stored, false otherwise (i.e. if the key already exists). </returns>
    bool SetValue(const char* key, void* value);
}
}
}
