#pragma once

#ifdef NAPA_MODULE_EXTENSION
#undef BUILDING_V8_SHARED
#define USING_V8_SHARED 1
#endif

#include "exports.h"

#include <array>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <v8.h>

namespace napa {
namespace module {

    /// <summary> Napa module version. </summary>
    static const int32_t MODULE_VERSION = 2;

    /// <summary> Variable name to export for module registration. </summary>
    static const char* MODULE_INFO_EXPORT = "_moduleInfo";

    /// <summary> Function pointer to initialize a module. It's called after a module is loaded. </summary>
    typedef void(*ModuleInitializer)(v8::Local<v8::Object> exports,
                                     v8::Local<v8::Value> module);

    /// <summary> Module information. </summary>
    struct ModuleInfo {
        /// <summary> Current module version. </summary>
        int32_t version;

        /// <summary> Module name. </summary>
        const char* moduleName;

        /// <summary> Function pointer to initialize a module. </summary>
        ModuleInitializer initializer;
    };

    /// <summary>
    /// Map from module's class name to persistent constructor object.
    /// To suppport multiple isolates, let each isolate has its own persistent constructor at thread local storage.
    /// </summary>
    typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function>> PersistentConstructor;
    struct ConstructorInfo {
        std::unordered_map<std::string, PersistentConstructor> constructorMap;
    };

    /// <summary> Isolate data Id to store Napa specific data for a module to be able to access. </summary>
    enum class IsolateDataId : uint32_t {
        /// <summary> Isolate instance. </summary>
        ISOLATE = 0,

        /// <summary> Module's persistent constructor object. </summary>
        CONSTRUCTOR,

        /// <summary> Module loader instance. </summary>
        MODULE_LOADER,

        /// <summary> End of index. </summary>
        END_OF_ISOLATE_DATA_ID
    };

    /// <summary> Napa specific isolate data stored at TLS. </summary>
    class NAPA_API IsolateData {
    public:

        /// <summary> Constructor to assign TLS index to all data. It's' done once at process level. </summary>
        IsolateData();

        /// <summary> Get stored TLS data. </summary>
        /// <param name="isolateDataId"> Pre-defined data id for Napa specific data. </param>
        /// <returns> Stored TLS data. </returns>
        static void* Get(IsolateDataId isolateDataId);

        /// <summary> Set TLS data into the given slot. </summary>
        /// <param name="isolateDataId"> Pre-defined data id for Napa specific data. </param>
        /// <param name="data"> Pointer to stored data. </param>
        static void Set(IsolateDataId isolateDataId,
                        void* data);

    private:

        /// <summary> It stores the tls index for all data. </summary>
        std::array<uint32_t, static_cast<size_t>(IsolateDataId::END_OF_ISOLATE_DATA_ID)> _tlsIndexes;
    };

    /// <summary> It binds the method name with V8 function. </summary>
    /// <param name="exports"> V8 object to bind with the given callback function. </param>
    /// <param name="name"> Method name. </param>
    /// <param name="callback"> Binding V8 function object. </param>
    template <typename T>
    void SetMethod(const T& exports,
                   const char* name,
                   v8::FunctionCallback callback) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);

        auto functionTemplate = v8::FunctionTemplate::New(isolate, callback);
        auto function = functionTemplate->GetFunction();
        auto functionName = v8::String::NewFromUtf8(isolate, name);
        function->SetName(functionName);

        exports->Set(functionName, function);
    }

    /// <summary> It binds the method name with V8 prototype function object. </summary>
    /// <param name="functionTemplate"> V8 function template object to bind with the given callback function. </param>
    /// <param name="name"> Method name. </param>
    /// <param name="callback"> Binding V8 function object. </param>
    inline void SetPrototypeMethod(v8::Local<v8::FunctionTemplate> functionTemplate,
                                   const char* name,
                                   v8::FunctionCallback callback) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);

        auto signature = v8::Signature::New(isolate, functionTemplate);
        auto signaturedFunctionTemplate = v8::FunctionTemplate::New(isolate, callback, v8::Local<v8::Value>(), signature);
        auto function = signaturedFunctionTemplate->GetFunction();

        functionTemplate->PrototypeTemplate()->Set(v8::String::NewFromUtf8(isolate, name), function);

        auto functionName = v8::String::NewFromUtf8(isolate, name);
        function->SetName(functionName);
    }

    #define NAPA_REGISTER_MODULE(name, function) \
        extern "C" { \
            _declspec(dllexport) napa::module::ModuleInfo _moduleInfo = { \
                napa::module::MODULE_VERSION, \
                #name, \
                reinterpret_cast<napa::module::ModuleInitializer>(function) \
            }; \
        }

    /// <summary> It sets the persistent constructor at the current V8 isolate. </summary>
    /// <param name="name"> Unique constructor name. It's recommended to use the same name as module. </param>
    /// <param name="constructor"> V8 persistent function to constructor V8 object. </param>
    inline void SetPersistentConstructor(const char* name,
                                         v8::Local<v8::Function> constructor) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::HandleScope scope(isolate);

        auto constructorInfo =
            static_cast<ConstructorInfo*>(IsolateData::Get(IsolateDataId::CONSTRUCTOR));
        if (constructorInfo == nullptr) {
            constructorInfo = new ConstructorInfo();
            IsolateData::Set(IsolateDataId::CONSTRUCTOR, constructorInfo);
        }

        constructorInfo->constructorMap.emplace(std::piecewise_construct,
                                                std::forward_as_tuple(name),
                                                std::forward_as_tuple(isolate, constructor));
    }

    /// <summary> It gets the given persistent constructor from the current V8 isolate. </summary>
    /// <param name="name"> Unique constructor name given at SetPersistentConstructor() call. </param>
    /// <returns> V8 local function object. </returns>
    inline v8::Local<v8::Function> GetPersistentConstructor(const char* name) {
        auto isolate = v8::Isolate::GetCurrent();
        v8::EscapableHandleScope scope(isolate);

        auto constructorInfo =
            static_cast<ConstructorInfo*>(IsolateData::Get(IsolateDataId::CONSTRUCTOR));
        if (constructorInfo == nullptr) {
            return scope.Escape(v8::Local<v8::Function>());
        }

        auto iter = constructorInfo->constructorMap.find(name);
        if (iter != constructorInfo->constructorMap.end()) {
            auto constructor = v8::Local<v8::Function>::New(isolate, iter->second);
            return scope.Escape(constructor);
        } else {
            return scope.Escape(v8::Local<v8::Function>());
        }
    }

    namespace global_store {

        /// <summary> Retrieve a value from the global store. </summary>
        /// <param name="key"> A key identifying the desired value. </param>
        /// <returns> The value if exists, nullptr otherwise. </returns>
        NAPA_API void* GetValue(const char* key);
    }

}   // End of namespace module.
}   // End of namespace napa.