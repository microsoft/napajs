#pragma once

#include <napa-module.h>
#include <shared-barrel.h>

namespace napa {
namespace module {

    /// <summary> Napa module of SharedBarrel Javascript wrapper. </summary>
    class SharedBarrelWrap : public NAPA_OBJECTWRAP {
    public:

        /// <summary> It registers this class into V8. </summary>
        static void Init();

        /// <summary> It creates the Javascript wrapper of shared barrel. </summary>
        /// <param name="args"> SharedBarrel instance. </param>
        static v8::Local<v8::Object> Create(std::shared_ptr<SharedBarrel> barrel);

        /// <summary> It returns shared barrel instance. </summary>
        std::shared_ptr<SharedBarrel> Get();

    private:

        /// <summary> Exported class name. </summary>
        static constexpr const char* _exportName = "SharedBarrelWrap";

        /// <summary> It creates V8 object at Javascript land. </summary>
        static void NewCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It creates V8 object at native land. </summary>
        static v8::Local<v8::Object> NewInstance();

        /// <summary> It returns shared barrel Id. </summary>
        static void GetId(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It returns true if a given key exists. </summary>
        /// <param name="args"> Shared object name. </parasm>
        static void Exists(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It adds a shared object into a shared barrel. </summary>
        /// <param name="args"> Shared object name and shared object. </parasm>
        static void Set(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It returns a Javascript wrapper of shared object from a shared barrel. </summary>
        /// <param name="args"> Shared object name. </parasm>
        static void Get(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It removes a shared object. </summary>
        /// <param name="args"> Shared object name. </parasm>
        static void Remove(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> It returns the number of shared objects in a shared barrel. </summary>
        static void Count(const v8::FunctionCallbackInfo<v8::Value>& args);

        /// <summary> Declare persistent constructor to create SharedBarrel Javascript wrapper instance. </summary>
        NAPA_DECLARE_PERSISTENT_CONSTRUCTOR

        std::shared_ptr<SharedBarrel> _barrel;
    };

}  // namespace module
}  // namespace napa