// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

/// <summary> Timer is a facility to run timed callbacks in caller's isolates. </summary>
export interface Timer {
}

const TIMEOUT_MAX = 2 ** 31 -1;

export class Timeout {
    _callback: (...args: any[]) => void;
    private _after: number;
    private _repeat: number;
    private _args: any[];
    _active: boolean;
    private _timer: Timer;

    constructor(callback: (...args: any[]) => void, 
                after: number, repeat: number, args: any[]) {
        if (after < 1) after = 0; //0 used for immediate
        if (after > TIMEOUT_MAX) after = 1;

        if (repeat < 1 || after == 0) repeat = 0; // do not repeat
        if (repeat > TIMEOUT_MAX) repeat = 1;

        this._callback = callback;
        this._after = after;
        this._repeat = repeat;
        this._args = args;
        this._timer = null;

        this._active = true;
    }
}

export type Immediate = Timeout;
