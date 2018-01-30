const napa = require(".");
const fs = require("fs");

const myUTFString="éóíúã’“";
console.log(myUTFString.length);

const zone = napa.zone.create("zone", {workers: 1});
const store = napa.store.create("store");
store.set("string", myUTFString);

zone.broadcast(`
    function example () {
        const str = global.napa.store.get("store").get("string");
        console.log(str.length);
        return str;
    }
`);

zone.execute("", "example").then(result => {
    console.log(result.value.length)
});