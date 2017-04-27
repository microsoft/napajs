#pragma once

#include <napa-module.h>

#include <memory>

// Forward declare zone.
namespace napa {
    class ZoneProxy;
}

namespace napa {
namespace binding {

    /// <summary> An object wrap to expose zone APIs. </summary>
    class ZoneWrap : public NAPA_OBJECTWRAP {
    public:

        enum class ConstructorType : uint32_t {
            CREATE = 0,  // Creates a new zone
            GET = 1,     // Retrieves an existing zone
            CURRENT = 2  // Retrieves the current zone
        };

        static void Init(v8::Isolate* isolate);
        static void NewInstance(ConstructorType type, const v8::FunctionCallbackInfo<v8::Value>& args);

    private:

        /// <summary> Declare persistent constructor to create Zone Javascript wrapper instance. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR;

        /// <summary> Exported class name. </summary>
        static constexpr const char* _exportName = "ZoneWrap";

        std::unique_ptr<napa::ZoneProxy> _zoneProxy;

        explicit ZoneWrap(std::unique_ptr<napa::ZoneProxy> zoneProxy);

        static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void GetId(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Broadcast(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void BroadcastSync(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Execute(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void ExecuteSync(const v8::FunctionCallbackInfo<v8::Value>& args);
    };
}
}