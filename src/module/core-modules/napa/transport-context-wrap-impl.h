// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <napa/module.h>
#include <napa/module/transport-context-wrap.h>

namespace napa {
namespace module {

    /// <summary> It implements napa::module::TransportContextWrap. </summary>
    /// <remarks> Reference: napajs/lib/transport/transportable.ts#TransportContext </remarks>
    class TransportContextWrapImpl : public napa::module::TransportContextWrap {
    public:
        /// <summary> Init this wrap. </summary>
        static void Init();

        /// <summary> Create a non-owning transport context wrap. </summary>
        static v8::Local<v8::Object> NewInstance(bool owning = true, napa::transport::TransportContext* context = nullptr);

        /// <summary> Destructor. </summary>
        ~TransportContextWrapImpl();

        /// <summary> Get transport context. </summary>
        napa::transport::TransportContext* Get() override;

        /// <summary> Declare constructor in public, so we can export class constructor to JavaScript world. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "TransportContextWrap";

    private:
        /// <summary> Constructor. </summary>
        TransportContextWrapImpl(napa::transport::TransportContext* context, bool owning);

        /// <summary> No copy allowed. </summary>
        TransportContextWrapImpl(const TransportContextWrapImpl&) = delete;
        TransportContextWrapImpl& operator=(const TransportContextWrapImpl&) = delete;

        /// <summary> TransportContextWrap.constructor </summary>
        static void ConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements TransportContext.sharedCount </summary>
        static void GetSharedCountCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& args);

        /// <summary> It implements TransportContext.saveShared(handle: Handle, object: napajs.memory.ShareableWrap) </summary>
        static void SaveSharedCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It implements TransportContext.loadShared(handle: Handle): napajs.memory.ShareableWrap) </summary>
        static void LoadSharedCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Transport context. </summary>
        napa::transport::TransportContext* _context;

        /// <summary> Own context or not. </summary>
        bool _owning;
    };
} // namespace module
} // namespace napa
