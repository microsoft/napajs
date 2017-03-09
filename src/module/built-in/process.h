#pragma once

#include <v8.h>

namespace napa {
namespace module {

/// <summary> Napa built-in addon to provide helper APIs for process. </summary>
namespace process {

    /// <summary> Set console object as global variable of given context. </summary>
    /// <param name="context"> V8 context for running javascript. </param>
    void Init(v8::Local<v8::Context> context);

}   // End of namespace process
}   // End of namespace module
}   // End of namespace napa
