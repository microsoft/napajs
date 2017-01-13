#ifndef NAPA_BINDING_CONTAINER_WRAP_H
#define NAPA_BINDING_CONTAINER_WRAP_H

#include <memory>

#include <node_object_wrap.h>

// Forward declare container.
namespace napa {
namespace runtime {
    class Container;
}
}

namespace napa {
namespace binding {

    /// <summary> A node object wrap to expose container APIs to node. </summary>
    class ContainerWrap : public node::ObjectWrap {
    public:
        static void Init(v8::Isolate* isolate);
        static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

    private:
        static v8::Persistent<v8::Function> _constructor;

        std::unique_ptr<napa::runtime::Container> _container;

        explicit ContainerWrap(std::unique_ptr<napa::runtime::Container> container);

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

#endif
