var binding = process.binding('tty_wrap');
var tty = exports;

tty.isatty = function(fd) {
    return binding.isTTY(fd);
}