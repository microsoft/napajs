// Export user types to a flattened namespace.
import * as objectModel from './object-model';
import * as builtins from './builtins';
import * as config from './config';
import * as utils from './utils'
import * as engine from './engine';

export {builtins, config, engine, objectModel, utils};
export * from './app'
export * from './wire'

import { Request, Response} from './wire';
import * as path from 'path';
import * as napa from 'napajs';

/// <summary> A global engine instance. </summary>
let _engine: engine.Engine = undefined;
let _engineSettings = config.EngineConfig.fromConfig(
    path.resolve(__dirname, "../config/engine.json"));

declare var __in_napa: boolean;

/// <summary> Initialize engine on demand. </summary>
function initEngine() {
    if (typeof __in_napa !== undefined) {
        // This module is loaded in NapaJS container.
        _engine = new engine.LocalEngine(_engineSettings);
    }
    else {
        // This module is loaded in Node.JS isolate.
        _engine = new engine.EngineHub(_engineSettings);
    }
}

/// <summary> Register a vinyard application. </summary>
/// <param name="appModuleName"> Module name for vinyard application, which contains an app.json under the path. </param>
/// <param name="appInstanceNames"> A list of names used to serve application, which correspond to 'application' property in Request. </param>
/// <param name="container"> Optional. Napa container to run the application, if not specified, run application in current V8 isolate. </param>
/// <exception> Throws Error if the module is not found or not a valid vinyard application. </exception>
export function register(
    appModuleName: string, 
    appInstanceNames: string[], 
    container: napa.Container = null): void {

    // Lazy creation of engine when register is called at the first time.
    if (_engine == null) {
        initEngine();
    }

    return _engine.register(
        appModuleName, 
        appInstanceNames, 
        container);
}

/// <summary> Serve a request with a promise of response. </summary>
/// <param name="request"> Requet in form of a JSON string or vinyard Request object. </param>
/// <returns> A promise of vinyard Response. This function call may be synchrous or asynchrous depending on the entrypoint. </returns>
export async function serve(
    request: string | Request): Promise<Response> {
    return await _engine.serve(request);
}

/// <summary> Get all application names served by current engine. </summary>
export function getApplicationInstanceNames(): string[] {
    return _engine.applicationInstanceNames;
}