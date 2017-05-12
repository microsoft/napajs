if (typeof __in_napa === 'undefined') {
    module.exports = require('../bin/napa-binding');
} else {
    module.exports = process.binding('napa-binding');
}
