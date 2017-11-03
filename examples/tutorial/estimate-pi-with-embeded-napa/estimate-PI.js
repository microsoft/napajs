
console.log('1234567890');

function estimatePI(points) {
    var i = points;
    var inside = 0;
    while (i-- > 0) {
        if (i%10000 == 0) console.log(i);
        var x = Math.random();
        var y = Math.random();
        if ((x * x) + (y * y) <= 1) {
            inside++;
        }
    }
        return inside / points * 4;
}

function run() {
    console.log('start run');

    var napa = require('napajs');
    console.log('napajs loaded');
    const NUMBER_OF_WORKERS = 4;
    var zone = napa.zone.create('zone1', { workers: NUMBER_OF_WORKERS });
    console.log('zone created with 4 workers');

    var promises = [];
    for (var i = 0; i < 4; i++) {
        promises[i] = zone.execute(estimatePI, [40000]);
    }

    console.log('4 times napa.zone.execution issued.');
    Promise.all(promises).then(values => {
        var aggregate = 0;
        values.forEach(result => aggregate += result.value);
        console.log('PI: ', aggregate / 4);
    });

    return 'returned-value-to-cpp-world';
}

run();
