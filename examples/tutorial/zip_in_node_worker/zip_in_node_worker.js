// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

"use strict";

const fs = require('fs');
const napa = require('../../../lib');

const {generate_zip_unzip} = require('./zip_function');

const numberOfFiles = 4;
const sizeInUuid = 32 * 1024;

if (process.argv.length > 2) {
    console.log("====Run generating/zipping in node main only...");
    for (let i=0; i < numberOfFiles; ++i) {
        const filename = `temp_content_000${i}.txt`;
        generate_zip_unzip(sizeInUuid, filename);
    }
}
else {
    console.log("====Run generating/zipping in napa's node worker...");
    const sampleZipZone = napa.zone.create('sampleZipZone', {workers : 1});
    for (let i=0; i < numberOfFiles; ++i) {
        const filename = `temp_content_000${i}.txt`;
        sampleZipZone.execute('./zip_function.js', 'generate_zip_unzip', [sizeInUuid, filename]);
    };
    sampleZipZone.on('terminated', () => {
	console.log("====zip zone exited."); 
    });
    sampleZipZone.recycle();
}

process.on('beforeExit', () => {
    for (let i=0; i < numberOfFiles; ++i) {
        const filename = `temp_content_000${i}.txt`;
        console.log(`Removing temp file ${filename} and ${filename}.gz ......`);
        try {
            fs.unlinkSync(filename);
            fs.unlinkSync(`${filename}.gz`);
        }
        catch (err) {
            console.log(err);
        }
    };
});

0;

