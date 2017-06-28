#pragma once

#include <napa-module.h>
#include <napa/module/common.h>
#include <napa/module/shareable-wrap.h>

#include <zone/call-context.h>

namespace napa {
namespace module {

    /// <summary> 
    ///     Class that wraps zone::CallContext, which enables JavaScript world to 
    ///     resolve or reject a function call.
    /// </summary>
    class CallContextWrap: public ShareableWrap {
    public:
        /// <summary> It creates a persistent constructor for CallContextWrap instance. </summary>
        static void Init();

        /// <summary> Create a new instance of wrap associating with specific call context. </summary>
        static v8::Local<v8::Object> NewInstance(std::shared_ptr<zone::CallContext> call);

        /// <summary> Get call context. </summary>
        zone::CallContext& GetRef();

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "CallContextWrap";

        /// <summary> Declare constructor in public, so we can export class constructor in JavaScript world. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

    protected:
        /// <summary> It implements CallContext.resolve(result: any): void </summary>
        static void ResolveCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements CallContext.reject(reason: string): void </summary>
        static void RejectCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements CallContext.finished: boolean </summary>
        static void IsFinishedCallback(v8::Local<v8::String> propertyName, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements CallContext.module: string </summary>
        static void GetModuleCallback(v8::Local<v8::String> propertyName, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements CallContext.function: string </summary>
        static void GetFunctionCallback(v8::Local<v8::String> propertyName, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements CallContext.args: string[] </summary>
        static void GetArgumentsCallback(v8::Local<v8::String> propertyName, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements CallContext.transportContext: TransportContext </summary>
        static void GetTransportContextCallback(v8::Local<v8::String> propertyName, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements CallContext.options: CallOptions </summary>
        static void GetOptionsCallback(v8::Local<v8::String> propertyName, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements CallContext.elapse: [number, number] (precision in nano-second) </summary>
        static void GetElapseCallback(v8::Local<v8::String> propertyName, const v8::PropertyCallbackInfo<v8::Value>& args);
    };
}
}
    