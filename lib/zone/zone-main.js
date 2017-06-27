var transport = require('../lib/transport');

function __zone_execute__(moduleName, functionName, args, transportContextHandle, options) {
    var module = null;
    if (moduleName == null || moduleName.length === 0) {
        module = this;
    } else if (moduleName !== '__function') {
        module = require(moduleName);
    }
    
    var func = null;
    if (module != null) {
        func = module;
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
    } else {
        // Anonymous function.
        func = transport.loadFunction(functionName);
    }

    var transportContext = transport.createTransportContext(transportContextHandle);
    var args = args.map((arg) => { return transport.unmarshall(arg, transportContext); });
    return transport.marshall(func.apply(this, args), transportContext);
}