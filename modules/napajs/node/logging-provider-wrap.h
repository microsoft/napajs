#pragma once

#include <napa-module.h>

namespace napa {
namespace binding {

    /// <summary> A class for creating a JS wrapper around napa logging provider. </summary>
    class LoggingProviderWrap {
    public:

        /// <summary> It registers this class into V8. </summary>
        static void Init();

        /// <summary> It creates V8 object at native land. </summary>
        static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

    private:

        /// <summary> Declare persistent constructor to create logging provider wrapper instance. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

        /// <summary> Exported class name. </summary>
        static constexpr const char* _exportName = "LoggingProviderWrap";

        /// <summary> It creates V8 object at Javascript land. </summary>
        static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Forwards the log to the confgiure napa logging provider. </summary>
        static void Log(const v8::FunctionCallbackInfo<v8::Value>& args);
    };

}  // namespace binding
}  // namespace napa