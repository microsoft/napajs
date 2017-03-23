#pragma once

#include <v8.h>

namespace napa {
namespace module {

/// <summary> Napa built-in addon to print a message to console. </summary>
namespace console {

    /// <summary> Set console object as global variable of given context. </summary>
    /// <param name="exports"> Object to set module. </param>
    void Init(v8::Local<v8::Object> exports);

}   // End of namespace console
}   // End of namespace module
}   // End of namespace napa
