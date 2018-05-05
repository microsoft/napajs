// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

// Suppress 4100 warnings.
#pragma warning(push)
#pragma warning(disable: 4100)

#include <node.h>
#include <node_object_wrap.h>

#include "napa/v8-helpers.h"
#include "napa/transport/object-factory.h"

#pragma warning(pop)

/// <summary> It binds a name with a property with V8 prototype function object. </summary>
#define NAPA_SET_PROTOTYPE_PROPERTY(functionTemplate, name, value) \
    functionTemplate->PrototypeTemplate()->Set(v8::Isolate::GetCurrent(), \
                                               name, \
                                               value)

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
                                              
/// <summary> It defines the persistent constructor. <summary>
/// <remarks> Napa does nothing since it registers a constructor while setting it. </remarks>
#define NAPA_DEFINE_PERSISTENT_CONSTRUCTOR(className) v8::Persistent<v8::Function> className::_constructor

/// <summary> It defines the template class's persistent constructor. <summary>
/// <remarks> Napa does nothing since it registers a constructor while setting it. </remarks>
#define NAPA_DEFINE_TEMPLATE_PERSISTENT_CONSTRUCTOR(className) \
    template <typename T> \
    v8::Persistent<v8::Function> className<T>::_constructor

/// <summary> It sets the persistent constructor at the current V8 isolate. </summary>
#define NAPA_SET_PERSISTENT_CONSTRUCTOR(name, function) \
    napa::objectFactory::RegisterConstrutor(name, function);

/// <summary> It gets the given persistent constructor from the current V8 isolate. </summary>
/// <returns> V8 local function object. </returns>
#define NAPA_GET_PERSISTENT_CONSTRUCTOR(exportName, className) \
    napa::objectFactory::GetConstructor(exportName)

/// <summary> It exports a node::ObjectWrap subclass to addon exports object. </summary>
#define NAPA_EXPORT_OBJECTWRAP(exports, exportName, className) \
    exports->Set(v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), exportName), \
                 NAPA_GET_PERSISTENT_CONSTRUCTOR(exportName, className))

// Depends on NAPA_GET_PERSISTENT_CONSTRUCTOR.
#include "napa/module/common.h"
