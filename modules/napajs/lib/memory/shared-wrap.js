var transport = require('../transport');
var addon = require('../../bin/addon');
transport.register(addon.SharedWrap);
exports.SharedWrap = addon.SharedWrap;
