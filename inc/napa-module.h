#pragma once

// Suppress 4100 warnings.
#pragma warning(push)
#pragma warning(disable: 4100)

#ifdef BUILDING_NAPA_EXTENSION
#include "napa/module/module-internal.h"
#include "napa/module/module-node-compat.h"
#include "napa/module/object-wrap.h"
#else
#include <node.h>
#include <node_object_wrap.h>
#endif

#include "napa/v8-helpers.h"

#pragma warning(pop)

/// <summary> It binds the method name with V8 function object. </summary>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_SET_METHOD napa::module::SetMethod
#else
#define NAPA_SET_METHOD NODE_SET_METHOD
#endif

/// <summary> It binds the method name with V8 prototype function object. </summary>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_SET_PROTOTYPE_METHOD napa::module::SetPrototypeMethod
#else
#define NAPA_SET_PROTOTYPE_METHOD NODE_SET_PROTOTYPE_METHOD
#endif

/// <summary> It binds a name with a property with V8 prototype function object. </summary>
#define NAPA_SET_PROTOTYPE_PROPERTY(functionTemplate, name, value) \
    functionTemplate->PrototypeTemplate()->Set(v8::Isolate::GetCurrent(), \
                                               name, \
                                               value)

/// <summary> It registers the module with the name and the initializer. </summary>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_MODULE NAPA_REGISTER_MODULE
#else
#define NAPA_MODULE NODE_MODULE
#endif

/// <summary> It wraps V8 object, so its lifetime can be managed by napa/node. </summary>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_OBJECTWRAP napa::module::ObjectWrap
#else
#define NAPA_OBJECTWRAP node::ObjectWrap
#endif

/// <summary> It sets the accessors for the given V8 function template object. </summary>
#define NAPA_SET_ACCESSOR(functionTemplate, name, getter, setter) \
    functionTemplate->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), name), \
                                                      getter, \
                                                      setter)

/// <summary> It sets the property for the given V8 function template object. </summary>
#define NAPA_SET_PROPERTY(functionTemplate, name, value) \
    functionTemplate->InstanceTemplate()->Set(v8::Isolate::GetCurrent(), \
                                              name, \
                                              value)

/// <summary> It declares the persistent constructor. </summary>
/// <remarks> Napa registers constructor at local thread storage. </remarks>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_DECLARE_PERSISTENT_CONSTRUCTOR
#else
#define NAPA_DECLARE_PERSISTENT_CONSTRUCTOR \
    static v8::Persistent<v8::Function> _constructor;
#endif

/// <summary> It defines the persistent constructor. <summary>
/// <remarks> Napa does nothing since it registers a constructor while setting it. </remarks>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(className)
#else
#define NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(className) \
    v8::Persistent<v8::Function> className::_constructor;
#endif

/// <summary> It defines the template class's persistent constructor. <summary>
/// <remarks> Napa does nothing since it registers a constructor while setting it. </remarks>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_DEFINE_TEMPLATE_PERSISTENT_CONSTRUCTOR(className)
#else
#define NAPA_DEFINE_TEMPLATE_PERSISTENT_CONSTRUCTOR(className) \
    template <typename T> \
    v8::Persistent<v8::Function> className<T>::_constructor;
#endif

/// <summary> It sets the persistent constructor at the current V8 isolate. </summary>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_SET_PERSISTENT_CONSTRUCTOR(name, function) \
    napa::module::SetPersistentConstructor(name, function)
#else
#define NAPA_SET_PERSISTENT_CONSTRUCTOR(name, function) \
    _constructor.Reset(v8::Isolate::GetCurrent(), function)
#endif

/// <summary> It gets the given persistent constructor from the current V8 isolate. </summary>
/// <returns> V8 local function object. </returns>
#ifdef BUILDING_NAPA_EXTENSION
#define NAPA_GET_PERSISTENT_CONSTRUCTOR(exportName, className) \
    napa::module::GetPersistentConstructor(exportName)
#else
#define NAPA_GET_PERSISTENT_CONSTRUCTOR(exportName, className) \
    v8::Local<v8::Function>::New(v8::Isolate::GetCurrent(), className::_constructor)
#endif

/// <summary> It exports a NAPA_OBJECTWRAP subclass to addon exports object. </summary>
#define NAPA_EXPORT_OBJECTWRAP(exports, exportName, className) \
    exports->Set(v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), exportName), \
                 NAPA_GET_PERSISTENT_CONSTRUCTOR(exportName, className))
