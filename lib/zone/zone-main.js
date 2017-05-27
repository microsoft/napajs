var transport = require('napajs/lib/transport');

function __zone_execute__(moduleName, functionName, args, contextHandle) {
    var module = (moduleName == null || moduleName.length === 0)? this : require(moduleName);
    var func = module;
    if (functionName != null && functionName.length != 0) {
        var path = functionName.split('.');
        for (item of path) {
            func = func[item];
            if (func === undefined) {
                throw new Error("Cannot find function '" + functionName + "' in module '" + moduleName + "'");
            }
        }
    }
    if (typeof func !== 'function') {
        throw new Error("'" + functionName + "' in module '" + moduleName + "' is not a function");
    }

    var transportContext = transport.createTransportContext(contextHandle);
    var args = args.map((arg) => { return transport.unmarshall(arg, transportContext); });
    return transport.marshall(func.apply(this, args), transportContext);
}