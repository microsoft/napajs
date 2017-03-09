#pragma once

#include <v8.h>

namespace napa {
namespace module {

/// <summary> Napa built-in addon to print a message to console. </summary>
namespace console {

    /// <summary> Set console object as global variable of given context. </summary>
    /// <param name="context"> V8 context for running javascript. </param>
    void Init(v8::Local<v8::Context> context);

}   // End of namespace console
}   // End of namespace module
}   // End of namespace napa
