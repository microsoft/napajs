import { EntryPoint, Interceptor, RequestContext } from './app';
import * as objectModel from './object-model';
import * as wire from './wire';
import * as utils from './utils';
import * as path from 'path';

////////////////////////////////////////////////////////////////////////
/// JSON definition for built-in object types.

/// <summary> Definition for function object. </summary>
export interface FunctionDefinition {
    // <summary> For referencing existing function. </summary>
    moduleName?: string,
    functionName?: string,

    /// <summary> for inline function. </summary>
    function?: string;
}

/// <summary> Entrypoint definition. </summary>
export interface EntryPointDefinition extends FunctionDefinition {
    /// <summary> _type === 'EntryPoint' </summary>
    _type: "EntryPoint",

    /// <summary> Optional. Description of entrypoint. </summary>
    description?: string,

    /// <summary> Optional. Custom execution stack of interceptor names. </summary>
    executionStack?: string[],

    /// <summary> Optional. Display rank. </summary>
    displayRank?: number,

    /// <summary> Optional. Example requests. This is for human consumption. </summary>.
    exampleRequests?: wire.Request[],

    /// <summary> Optional. Example responses. </summary>
    exampleResponses?: wire.Response[]
};

/// <summary> Interceptor definition. </summary>
export interface InterceptorDefinition extends FunctionDefinition {
    /// <summary> _type === 'Interceptor' </summary>
    _type: "Interceptor",

    /// <summary> Optional. Description of interceptor </summary>
    description?: string,
};

////////////////////////////////////////////////////////////////////////////////
/// Object constructors for built-in objects.

/// <summary> Constructor for Function. </summary>
export function createFunction(
    definition: FunctionDefinition, 
    context: objectModel.ObjectContext): Function {

    if (definition.function != null) {
        // Dynamicly created function.
        // TODO: do security check. 
        return eval('(' + definition.function + ')');
    }

    if (definition.moduleName != null && definition.functionName != null) {
        // create function from module and function name.
        let moduleName = definition.moduleName;
        if (moduleName.startsWith('.')) {
            moduleName = path.resolve(context.baseDir, moduleName);
        }
        return utils.appendMessageOnException("Unable to create function '" 
                + definition.function 
                + "' in module '" 
                + definition.moduleName 
                + "'.", 
                () => {
                    return utils.loadFunction(moduleName, definition.functionName);
                });
    }
    throw new Error("Either property group 'moduleName' and 'functionName' or property 'function' should be present for Function object.");
}

/// <summary> Constructor for EntryPoint. </summary>
export function createEntryPoint(
    definition: EntryPointDefinition,
    context: objectModel.ObjectContext): EntryPoint {
    // TODO: any check?
    return <EntryPoint>createFunction(definition, context);
}

/// <summary> Constructor for Interceptor. </summary>
export function createInterceptor(
    definition: Interceptor,
    context: objectModel.ObjectContext): Interceptor {
    // TODO: any check?
    return <Interceptor>createFunction(definition, context);
}

