#pragma once

// Suppress 4100 warnings.
#pragma warning(push)
#pragma warning(disable: 4100)

#ifdef NAPA_MODULE_EXTENSION
#include "napa/module-internal.h"
#include "napa/module-node-compat.h"
#include "napa/object-wrap.h"
#else
#include <node.h>
#include <node_object_wrap.h>
#endif

#include "napa/v8-helpers.h"

#pragma warning(pop)

/// <summary> It binds the method name with V8 function object. </summary>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_SET_METHOD napa::module::SetMethod
#else
#define NAPA_SET_METHOD NODE_SET_METHOD
#endif

/// <summary> It binds the method name with V8 prototype function object. </summary>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_SET_PROTOTYPE_METHOD napa::module::SetPrototypeMethod
#else
#define NAPA_SET_PROTOTYPE_METHOD NODE_SET_PROTOTYPE_METHOD
#endif

/// <summary> It registers the module with the name and the initializer. </summary>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_MODULE NAPA_REGISTER_MODULE
#else
#define NAPA_MODULE NODE_MODULE
#endif

/// <summary> It sets the accessors for the given V8 function template object. </summary>
#define NAPA_SET_ACCESSOR(functionTemplate, name, getter, setter) \
    functionTemplate->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(isolate, name), \
                                                      getter, \
                                                      setter);

/// <summary> It wraps V8 object, so its lifetime can be managed by napa/node. </summary>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_OBJECTWRAP napa::module::ObjectWrap
#else
#define NAPA_OBJECTWRAP node::ObjectWrap
#endif

/// <summary> It declares the persistent constructor. </summary>
/// <remarks> Napa registers constructor at local thread storage. </remarks>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_DECLARE_PERSISTENT_CONSTRUCTOR
#else
#define NAPA_DECLARE_PERSISTENT_CONSTRUCTOR \
    static v8::Persistent<v8::Function> _constructor;
#endif

/// <summary> It defines the persistent constructor. <summary>
/// <remarks> Napa does nothing since it registers a constructor while setting it. </remarks>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(Class)
#else
#define NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(Class) \
    v8::Persistent<v8::Function> Class::_constructor;
#endif

/// <summary> It defines the template class's persistent constructor. <summary>
/// <remarks> Napa does nothing since it registers a constructor while setting it. </remarks>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_DEFINE_TEMPLATE_PERSISTENT_CONSTRUCTOR(Class)
#else
#define NAPA_DEFINE_TEMPLATE_PERSISTENT_CONSTRUCTOR(Class) \
    template <typename T> \
    v8::Persistent<v8::Function> Class<T>::_constructor;
#endif

/// <summary> It sets the persistent constructor at the current V8 isolate. </summary>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_SET_PERSISTENT_CONSTRUCTOR(name, function) \
    napa::module::SetPersistentConstructor(name, function);
#else
#define NAPA_SET_PERSISTENT_CONSTRUCTOR(name, function) \
    _constructor.Reset(v8::Isolate::GetCurrent(), function);
#endif

/// <summary> It gets the given persistent constructor from the current V8 isolate. </summary>
/// <returns> V8 local function object. </returns>
#ifdef NAPA_MODULE_EXTENSION
#define NAPA_GET_PERSISTENT_CONSTRUCTOR(name) \
    napa::module::GetPersistentConstructor(name);
#else
#define NAPA_GET_PERSISTENT_CONSTRUCTOR(name) \
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), _constructor);
#endif