var assert = require("assert");

if (process.version >= 'v9.0.0' || process.version.indexOf('.') > 2) {
    var napa = require("../lib/index");
    var zone = napa.zone.create('zone', { workers: 4 });
    var promises = [];
    var sab = new SharedArrayBuffer(4);
    for (var i = 0; i < 4; i++) {
        promises[i] = zone.execute((sab, i) => {
            var ta = new Uint8Array(sab);
            ta[i] = 100;
            return i;
        }, [sab, i]);
    }

    return Promise.all(promises).then(values => {
        var ta = new Uint8Array(sab);
        assert.deepEqual(ta.toString(), '100,100,100,100');
    });
}