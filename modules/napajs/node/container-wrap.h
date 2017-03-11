#pragma once

#include <napa-module.h>

#include <memory>

// Forward declare container.
namespace napa {
    class Container;
}

namespace napa {
namespace binding {

    /// <summary> A node object wrap to expose container APIs to node. </summary>
    class ContainerWrap : public NAPA_OBJECTWRAP {
    public:
        static void Init(v8::Isolate* isolate);
        static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

    private:

        /// <summary> Declare persistent constructor to create Container Javascript wrapper instance. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR;

        /// <summary> Exported class name. </summary>
        static constexpr const char* _exportName = "ContainerWrap";

        std::unique_ptr<napa::Container> _container;

        explicit ContainerWrap(std::unique_ptr<napa::Container> container);

        static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void LoadSync(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void LoadFile(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void LoadFileSync(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void Run(const v8::FunctionCallbackInfo<v8::Value>& args);
        static void RunSync(const v8::FunctionCallbackInfo<v8::Value>& args);
    };
}
}