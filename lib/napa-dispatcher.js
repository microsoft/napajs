function __napa_function_dispatcher__(func, args, contextHandle) {
    var transport = require('napajs').transport;
    
    var transportContext = transport.createTransportContext(contextHandle);
    var args = args.map((arg) => { return transport.unmarshall(arg, transportContext); });

    return transport.marshall(func.apply(this, args), transportContext);
}

function __napa_module_dispatcher__(moduleName, functionName, args, contextHandle) {
    var module = require(moduleName);
    var func = module[functionName];
    if (!func) {
        throw new Error("Cannot find function '" + functionName + "' in module '" + moduleName + "'");
    }

    return __napa_function_dispatcher__(func, args, contextHandle);
}