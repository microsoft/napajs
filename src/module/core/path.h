#pragma once

#include <v8.h>

namespace napa {
namespace module {

/// <summary> Napa built-in addon for path operations. </summary>
namespace path {

    /// <summary> Set path object. </summary>
    /// <param name="exports"> Object to set module. </param>
    void Init(v8::Local<v8::Object> exports);

}   // End of namespace path
}   // End of namespace module
}   // End of namespace napa
