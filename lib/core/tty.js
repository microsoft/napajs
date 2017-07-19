// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

var binding = process.binding('tty_wrap');
var tty = exports;

tty.isatty = function(fd) {
    return binding.isTTY(fd);
}