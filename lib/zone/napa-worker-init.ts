// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

const __in_napa: boolean = true;
(<any>global).__in_napa = __in_napa;

const __zone_id: string = process.argv[2];
(<any>global).__zone_id = __zone_id;

const __worker_id = process.argv[3];
(<any>global).__worker_id = __worker_id;

require('../../');

(<any>global).__recycle = (function () {
    const aliveInterval = 30000;
    let handle = setInterval(function () {
    }, aliveInterval);

    return function () {
        clearInterval(handle);
    };
})();
