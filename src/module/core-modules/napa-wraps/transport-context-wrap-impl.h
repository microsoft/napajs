#pragma once

#include <napa-module.h>
#include <napa/module/transport-context-wrap.h>

namespace napa {
namespace module {
    
    /// <summary> It implements napa::module::TransportContextWrap. </summary>
    /// <remarks> Reference: napajs/lib/transport/transportable.ts#TransportContext </remarks>
    class TransportContextWrapImpl: public napa::module::TransportContextWrap {
    public:
        /// <summary> Init this wrap. </summary>
        static void Init();

        /// <summary> Get transport context. </summary>
        napa::transport::TransportContext* Get() override;

        /// <summary> Declare constructor in public, so we can export class constructor to JavaScript world. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

    private:
        /// <summary> Constructor. </summary>
        TransportContextWrapImpl(napa::transport::TransportContext* context);

        /// <summary> No copy allowed. </summary>
        TransportContextWrapImpl(const TransportContextWrapImpl&) = delete;
        TransportContextWrapImpl& operator=(const TransportContextWrapImpl&) = delete;

        /// <summary> TransportContextWrap.constructor </summary>
        static void ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements TransportContext.sharedCount </summary>
        static void GetSharedCountCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements TransportContext.saveShared(handle: Handle, object: napajs.memory.SharedWrap) </summary>
        static void SaveSharedCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements TransportContext.loadShared(handle: Handle): napajs.memory.SharedWrap) </summary>
        static void LoadSharedCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Exported class name. </summary>
        static constexpr const char* _exportName = "TransportContextWrap";

        /// <summary> Non-owning transport context. </summary>
        napa::transport::TransportContext* _context;
    };
}
}
