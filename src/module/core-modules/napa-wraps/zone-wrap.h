#pragma once

#include <napa-module.h>
#include <napa/module/common.h>

#include <memory>

// Forward declare zone.
namespace napa {
    class ZoneProxy;
}

namespace napa {
namespace module {

    /// <summary> An object wrap to expose zone APIs. </summary>
    class ZoneWrap : public NAPA_OBJECTWRAP {
    public:

        /// <summary> Exported class name. </summary>
        static constexpr const char* exportName = "ZoneWrap";

        /// <summary> Initializes the wrap. </summary>
        static void Init();

        /// <summary> Create a new ZoneWrap instance that wraps the provided proxy. </summary>
        static v8::Local<v8::Object> NewInstance(std::unique_ptr<napa::ZoneProxy> zoneProxy);

    private:

        /// <summary> Declare persistent constructor to create Zone Javascript wrapper instance. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR;

        std::unique_ptr<napa::ZoneProxy> _zoneProxy;

        // ZoneWrap methods
        static void GetId(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Broadcast(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void BroadcastSync(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Execute(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void ExecuteSync(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Friend default constructor callback. </summary>
        template <typename WrapType>
        friend void napa::module::DefaultConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>&);
    };
}
}