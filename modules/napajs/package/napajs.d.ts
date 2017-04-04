
import logger = require('@napajs/logger');

export declare function initialize(): void;
export declare function initialize(settings: string): void;
export declare function initialize(settings: object): void;
export declare function shutdown(): void;

type ResponseCode = number;

export declare class Response {
    code: ResponseCode;
    errorMessage: string;
    returnValue: string;
}

export interface Container {
    load(source: string, callback: (responseCode: ResponseCode) => void): void;
    loadSync(source: string): ResponseCode;

    loadFile(file: string, callback: (responseCode: ResponseCode) => void): void;
    loadFileSync(file: string): ResponseCode;

    run(functionName: string, args: string[], callback: (response: Response) => void, timeout?: number): void;
    runSync(functionName: string, args: string[], timeout?: number): Response;

    runAll(functionName: string, args: string[], callback: (responseCode: ResponseCode) => void): void;
    runAllSync(functionName: string, args: string[]): ResponseCode;
}

export declare function createContainer(settings?: object): Container;

export declare function getLoggingProvider(): logger.LoggingProvider;

export declare function getResponseCodeString(code: number): string;
