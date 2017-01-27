#ifndef NAPA_MODULE_NODE_COMPAT_H
#define NAPA_MODULE_NODE_COMPAT_H

/// <summary> It binds the method name with V8 function object. </summary>
#define NODE_SET_METHOD napa::module::SetMethod

/// <summary> It binds the method name with V8 prototype function object. </summary>
#define NODE_SET_PROTOTYPE_METHOD napa::module::SetPrototypeMethod

/// <summary> It registers the module with the name and the initializer. </summary>
#define NODE_MODULE NAPA_REGISTER_MODULE

#endif // NAPA_MODULE_NODE_COMPAT_H
