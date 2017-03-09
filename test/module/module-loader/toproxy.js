var exports = module.exports = {};

exports.run = function(data) {
    var toupper = require('./toupper');
    return toupper.toupper(data);
};
