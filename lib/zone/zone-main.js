var transport = require('napajs/lib/transport');

function __zone_function_main__(func, args, contextHandle) {    
    var transportContext = transport.createTransportContext(contextHandle);
    var args = args.map((arg) => { return transport.unmarshall(arg, transportContext); });

    return transport.marshall(func.apply(this, args), transportContext);
}

function __zone_module_main__(moduleName, functionName, args, contextHandle) {
    var module = require(moduleName);
    var func = module[functionName];
    if (!func) {
        throw new Error("Cannot find function '" + functionName + "' in module '" + moduleName + "'");
    }

    return __zone_function_main__(func, args, contextHandle);
}