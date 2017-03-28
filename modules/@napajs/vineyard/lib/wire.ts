import * as objectModel from './object-model';
import * as utils from './utils';
import * as path from 'path';

/////////////////////////////////////////////////////////////////////////////////////////////////
// Interfaces for vineyard wire format.

/// <summary> Interface for control flags. </summary>
export type ControlFlags = {
    /// <summary> Enable debugging or not. </summary>
    debug?: boolean;

    /// <summary> Return performance numbers or not. </summary>
    perf?: boolean;
}

/// <summary> Interface for vineyard request. </summary>
export interface Request {
    /// <summary> Application name </summary>
    application: string;

    /// <summary> Entry point name </summary>
    entryPoint: string;

    /// <summary> Trace ID </summary>
    traceId?: string;

    /// <summary> Input JS object for entry point </summary>
    input?: any;

    /// <summary> Control flags </summary>
    controlFlags?: ControlFlags;

    /// <summary> Overridden types </summary>
    overrideTypes?: objectModel.TypeDefinition[];

    /// <summary> Overridden named objects </summary>
    overrideObjects?: objectModel.NamedObjectDefinition[];

    /// <summary> Overridden providers </summary>
    overrideProviders?: objectModel.ProviderDefinition[];
}

/// <summary> Response code </summary>
export enum ResponseCode {
    // Success.
    Success = 0,

    // Internal error.
    InternalError = 1,

    // Server side timeout.
    ProcessTimeout = 2,

    // Throttled due to policy.
    Throttled = 3,

    // Error caused by bad input.
    InputError = 4
}

/// <summary> Exception information in response. </summary>
export type ExceptionInfo = {
    stack: string;
    message: string;
    fileName?: string;
    lineNumber?: number;
    columnNumber?: number;
}

/// <summary> Debug event in DebugInfo. </summary>
export type DebugEvent = {
    eventTime: Date;
    logLevel: string;
    message: string;
}

/// <summary> Debug information when debug flag is on. </summary>
export type DebugInfo = {
    exception: ExceptionInfo;
    events: DebugEvent[];
    details: { [key: string]: any };
    machineName: string;
}

/// <summary> Write performance numbers when perf flag is on. </summary>
export type PerfInfo = {
    processingLatencyInMS: number;
}

/// <summary> Interface for response </summary>
export interface Response {
    /// <summary> Response code </summary>
    responseCode: ResponseCode;

    /// <summary> Error message if response code is not Success. </summary>
    errorMessage?: string;

    /// <summary> Output from entrypoint. </summary>
    output?: any;

    /// <summary> Debug information. </summary>
    debugInfo?: DebugInfo;

    /// <summary> Performance numbers. </summary>
    perfInfo?: PerfInfo;
}

/// <summary> Request helper. </summary>
export class RequestHelper {
    /// <summary> JSON schema for resquest. </summary>
    private static readonly REQUEST_SCHEMA: utils.JsonSchema = new utils.JsonSchema(
        path.resolve(path.resolve(__dirname, '../schema'), "request.schema.json"));

    /// <summary> Set default values transform. </summary>
    private static _transform = new utils.SetDefaultValue({
        traceId: "Unknown",
        overrideObjects: [],
        overrideProviders: [],
        overrideTypes: [],
        controlFlags: {
                debug: false,
                perf: false
            }
    });

    /// <summary> Tell if a jsValue is a valid request at run time. </summary>
    public static validate(jsValue: any): boolean {
        return this.REQUEST_SCHEMA.validate(jsValue);
    }

    /// <summary> Create request from a JS value that conform with request schema. </summary>
    public static fromJsValue(jsValue: any): Request {
        if (!this.validate(jsValue))
            throw new Error("Request doesn't match request schema.");
        
        let request = <Request>(jsValue);
        this._transform.apply(request);
        // TODO: @dapeng, make SetDefaultValue recursive.
        if (request.controlFlags.debug == null) {
            request.controlFlags.debug = false;
        }

        if (request.controlFlags.perf == null) {
            request.controlFlags.perf = false;
        }

        return request;
    }
}

/// <summary> Response helper. </summary>
export class ResponseHelper {
    /// <summary> JSON schema for response. </summary>
    private static readonly RESPONSE_SCHEMA: utils.JsonSchema = new utils.JsonSchema(
        path.resolve(path.resolve(__dirname, '../schema'), "response.schema.json"));

    /// <summary> Parse a JSON string that conform with response schema. </summary>
    public static parse(jsonString: string): Response {
        let response = utils.parseJsonString(jsonString, this.RESPONSE_SCHEMA);
        return <Response>(response);
    }

    /// <summary> Validate a JS value against response schema. </summary>
    public static validate(jsValue: any): boolean {
        return this.RESPONSE_SCHEMA.validate(jsValue);
    }
}