#pragma once

namespace napa {
namespace v8_common {

    /// <summary> Performs v8 global initialization. </summary>
    bool Initialize();

    /// <summary> Shutdown and clean v8 global resources. </summary>
    void Shutdown();
}
}
