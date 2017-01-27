#ifndef NAPA_MODULE_NODE_COMPAT_H
#define NAPA_MODULE_NODE_COMPAT_H

/// <summary> It binds the method name with V8 function object. </summary>
/// <remarks> It's node macro, so provide the same macro name at Napa. </summary.
#ifdef NAPA_MODULE_EXTENSION
#define NODE_SET_METHOD napa::module::SetMethod
#endif

/// <summary> It binds the method name with V8 prototype function object. </summary>
/// <remarks> It's node macro, so provide the same macro name at Napa. </summary.
#ifdef NAPA_MODULE_EXTENSION
#define NODE_SET_PROTOTYPE_METHOD napa::module::SetPrototypeMethod
#endif

/// <summary> It registers the module with the name and the initializer. </summary>
/// <remarks> It's node macro, so provide the same macro name at Napa. </summary.
#ifdef NAPA_MODULE_EXTENSION
#define NODE_MODULE NAPA_REGISTER_MODULE
#endif

#endif // NAPA_MODULE_NODE_COMPAT_H