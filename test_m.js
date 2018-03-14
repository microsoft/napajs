"use strict";
Object.defineProperty(exports, "__esModule", { value: true });

exports.foo = () => {
    //xxx;
    console.log(Buffer.alloc(100));
    console.log('........zone.execute.....');
    setTimeout(() => {console.log('.......setTimeout Callback.....');}, 100);
    console.log('........after setTimeout.....');
    //console.log(global.require);
    //console.log(Buffer.alloc(100));
    //console.log(require);
    //Buffer.alloc(10);
};