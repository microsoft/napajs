import * as logger from '@napajs/logger';
import * as metrics from '@napajs/metrics';

import * as app from './app';
import * as wire from './wire';
import * as utils from './utils';


/////////////////////////////////////////////////////////////////
/// Built-in interceptors.

/// <summary> Interceptor: pass through.
/// This interceptor is used for debug purpose when doing per-request override
/// <summary> 
export async function passThrough(
    context: app.RequestContext): Promise<wire.Response> {
    return await context.continueExecution();
}

/// <summary> Interceptor: short circuit. 
/// This interceptor is used for debug purpose when doing per-request override
/// <summary> 
export async function shortCircuit(
    context: app.RequestContext): Promise<wire.Response> {
    return Promise.resolve({
        responseCode: wire.ResponseCode.Success
    });
}

/// <summary> Interceptor: execute entryPoint </summary>
export async function executeEntryPoint(
    context: app.RequestContext): Promise<wire.Response> {

    let response = await context.continueExecution();
    response.output = await utils.makePromiseIfNotAlready(
            context.entryPoint(context, context.input));

    return response;
}

/// <summary> Interceptor: log request only. </summary>
export async function logRequest(
    context: app.RequestContext): Promise<wire.Response> {

    logger.debug(JSON.stringify(context.request));
    return await context.continueExecution();
}

/// <summary> Interceptor: log response only. </summary>
export async function logResponse(
    context: app.RequestContext): Promise<wire.Response> {
    
    let response = await context.continueExecution();
    logger.debug(JSON.stringify(response));
    return response;
}

/// <summary> Interceptor: log request and response. </summary>
export async function logRequestResponse(
    context: app.RequestContext): Promise<wire.Response> {

    logger.debug(JSON.stringify(context.request));
    let response = await context.continueExecution();
    logger.debug(JSON.stringify(response));
    return response;
}

/// <summary> Interceptor: finalize response </summary>
export async function finalizeResponse(
    context: app.RequestContext): Promise<wire.Response> {

    let startTime = metrics.now();
    let response = await context.continueExecution();

    // Attach debug info if needed.
    if (context.controlFlags.debug) {
        response.debugInfo = context.debugger.getOutput();
    }

    // Attach perf info if needed.
    if (context.controlFlags.perf) {
        response.perfInfo = {
            processingLatencyInMS : metrics.elapseSince(startTime)
        };
    }
    return response;
}
