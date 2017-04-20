var transport = require('../transport');
var addon = require('../../bin/addon');
transport.register(addon.SharedWrap);
export var SharedWrap = addon.SharedWrap;
