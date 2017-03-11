
import logger = require('@napajs/logger');

export declare function initialize(): void;
export declare function initialize(settings: string): void;
export declare function initialize(settings: object): void;
export declare function shutdown(): void;

export declare class Response {
    code: number;
    errorMessage: string;
    returnValue: string;
}

export interface Container {
    load(source: string, callback: (responseCode: number) => void): void;
    loadSync(source: string): number;

    loadFile(file: string, callback: (responseCode: number) => void): void;
    loadFileSync(file: string): number;

    run(functionName: string, args: string[], callback: (response: Response) => void, timeout?: number): void;
    runSync(functionName: string, args: string[], timeout?: number): Response;
}

export declare function createContainer(settings?: object): Container;

export declare function getLoggingProvider(): logger.LoggingProvider;
