var exports = module.exports = {};

exports.print = function(data) {
    var proxy = require('./test/toproxy');
    return proxy.run(data);
};
