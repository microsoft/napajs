var napa = require('.');
var z1 = napa.zone.create('z1', { workers: 1 });

//z1.broadcastSync('');

// z1.broadcast('', 'eval', ['global.a = 123; console.log("inside");']).then(function () {
//     console.log("++++++++++ " + global.a);
// }, function(err) {
//     console.log('TTTTTTTTTT ' + err);
// });

napa.zone.current.broadcast(function () {
    console.log(z1.id);
}).then(function (res) {
    console.log(z1.id);

}).catch(function (err) {
    console.log(z1.id);
});

z1.broadcastSync(function () {
    var napa = require('.');
    //napa.zone.get('z1').broadcastSync("var a = 0;");
    napa.zone.node.broadcast("var a = 0;");
});

//napa.zone.node.broadcastSync("var a = 0;");
