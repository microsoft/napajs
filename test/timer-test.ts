// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// To Run this test, in napajs root directory after build, use:
//      node test/timer-test 

import * as assert from "assert";
import * as path from "path";
import * as napa from "../lib/index";

// To be execute in napa workers
export function setImmediateTest(taskGroupId: string) : Promise<string> {
    const kTaskGroupSize = 3;
    //TODO: where to make it global?
    let setImmediate = napa.timer.setImmediate;
    let clearImmediate = napa.timer.clearImmediate;

    let correctResult = "";
    let lastTaskString = "";
    for (let taskId = 0; taskId < kTaskGroupSize; taskId++) {
        let taskString = "" + taskGroupId + "-" + taskId;
        if (taskId != 1) {
            correctResult = correctResult + taskString + " : ";
            lastTaskString = taskString;
        }
    }


    var promise = new Promise<string>((resolve, reject) => {
        let execResult = "";
        for (let taskId = 0; taskId < kTaskGroupSize; taskId++) {
            let taskString = "" + taskGroupId + "-" + taskId;
            console.log("  ++Task:" +  taskString + " setting immediate call back");
            let immedidate = setImmediate((taskString: string) => {
                console.log("    Running Task:" + taskString + " in set immedidate call back");
                execResult = execResult + taskString + " : ";
                if (taskString == lastTaskString) {
                    if (execResult == correctResult) {
                        resolve(execResult)
                    }
                    else {
                        reject(execResult)
                    }
                }
            }, [taskString]);
            console.log("  ++Task:" + taskString + " setted immediate call back");
    
            if (taskId == 1) {
                console.log("  --Task:" + taskString + " clearing immediate call back");
                clearImmediate(immedidate);
                console.log("  --Task:" + taskString + " cleared immediate call back");
            }
        }
    });
    return promise;
}

export function setTimeoutTest(taskGroupId: string) {
    const kTaskGroupSize = 3;
    //TODO: where to make it global?
    let setTimeout = napa.timer.setTimeout;
    let clearTimeout = napa.timer.clearTimeout;

    for (let taskId = 0; taskId < kTaskGroupSize; taskId++) {
        let taskString = "" + taskGroupId + "-" + taskId;
        let wait = 100 * ((+taskGroupId) * kTaskGroupSize + taskId);
        console.log("  ++Task:" +  taskString + " setting timeout call back");
        let timeout = setTimeout((taskString: string) => {
            console.log("    Running Task:" + taskString + " in set timeout call back");
        }, wait, [taskString]);
        console.log("  ++Task:" + taskString + " setted timeout call back");

        if (taskId == 1) {
            console.log("  --Task:" + taskString + " clearing timeout call back");
            clearTimeout(timeout);
            console.log("  --Task:" + taskString + " cleared timeout call back");
        }
    }
}


export function setIntervalTest(taskGroupId: string, duration: number, count: number) {
    //TODO: where to make it global?
    let setInterval = napa.timer.setInterval;
    let clearInterval = napa.timer.clearInterval;
    let setTimeout = napa.timer.setTimeout;
    let clearTimeout = napa.timer.clearTimeout;

    console.log("  Interval Task:" + taskGroupId + " trying to run every " + duration + "ms for " + count + " times...");

    let interval = setInterval((taskString: string) => {
        console.log("    ---- Task:" + taskString + " in set interval call back");
    }, duration, [taskGroupId])

    setTimeout((taskString: String)=> { 
        console.log("    ---- Task:" + taskString + " clearing interval...");
        clearInterval(interval);
    }, duration * count, [taskGroupId]);
}


declare var __in_napa: boolean;

if (typeof __in_napa === 'undefined') {
    // Change this value to control number of napa workers initialized.
    const NUMBER_OF_WORKERS = 2;
    const kTaskGroupCount = 5;
    let zone = napa.zone.create('zone', { workers: NUMBER_OF_WORKERS });

    zone.broadcast('')
        .then(null, (err)=>{
            console.log("Error zone broadcast: " + err);
        })

    console.log("Waiting zone initalization......");
    setTimeout(()=>{
        console.log("\nStart setImmediate/clearImmediate test......");
        for (let groupId = 0; groupId < kTaskGroupCount; groupId++) {
            zone.execute('./timer-test', 'setImmediateTest', [groupId.toString()])
            .then(
                (res)=>{ console.log("\n\n=============\nCorrect result:" + res.toString()); },
                (err) =>{ console.log("\n\n=============\nWrong result:"+ err.toString()); }
            );
        }
    }, 300);


    setTimeout(()=>{
        console.log("\nStarting setTimeout/clearTimerout testing......");
        for (let groupId = 0; groupId < kTaskGroupCount; groupId++) {
            zone.execute('./timer-test', 'setTimeoutTest', [groupId.toString()]);
        }
    }, 1200);


    setTimeout(()=>{
        console.log("\nStarting setInterval/clearInterval testing......");
        for (let groupId = 0; groupId < 2; groupId++) {
            zone.execute('./timer-test', 'setIntervalTest', [groupId.toString(), 300*(groupId+1), 6]);
        }
    }, 3200);

    setTimeout(()=>{
        console.log("\nFinished waiting all timer test run.");
    }, 9900);
}




