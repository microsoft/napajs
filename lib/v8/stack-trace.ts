// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

/// Reference: https://github.com/v8/v8/wiki/Stack-Trace-API

/// <summary> Represents a stack frame. </summary>
export interface CallSite {

    /// <summary> Returns the value of this. </summary>
    getThis(): any;

    /// <summary> 
    ///     Returns the type of this as a string. 
    ///     This is the name of the function stored in the constructor field of this, if available, 
    ///     otherwise the object's [[Class]] internal property.
    /// </summary>
    getTypeName(): string;

    /// <summary> Returns the current function. </summary>
    getFunction(): any;

    /// <summary> 
    ///     Returns the name of the current function, typically its name property. 
    ///     If a name property is not available an attempt will be made to try to infer a name from the function's context. 
    /// </summary>
    getFunctionName(): string;

    /// <summary> Returns the name of the property of this or one of its prototypes that holds the current function. </summary>
    getMethodName(): string;

    /// <summary> If this function was defined in a script returns the name of the script. </summary>
    getFileName(): string;

    /// <summary> If this function was defined in a script returns the current line number. </summary>
    getLineNumber(): number;

    /// <summary> If this function was defined in a script returns the current column number. </summary>
    getColumnNumber(): number;

    /// <summary> If this function was created using a call to eval returns a CallSite object representing the location where eval was called. </summary>
    getEvalOrigin(): CallSite;

    /// <summary> Is this a toplevel invocation, that is, is this the global object. </summary>
    isToplevel(): boolean;

    /// <summary> Is this call in native V8 code. </summary>
    isNative(): boolean;

    /// <summary> Is this a constructor call. </summary>
    isConstructor(): boolean;

    /// <summary> Does this call take place in code defined by a call to eval. </summary>
    isEval(): boolean;
}

/// <summary> Get current stack. </summary>
/// <param name="stackTraceLimit"> Max stack depth to trace. </param>
/// <returns> Array of CallSite. </returns>
export function currentStack(stackTraceLimit: number = 0): CallSite[] {
    let e: any = Error;

    const originPrepare = e.prepareStackTrace;
    const originLimit = e.stackTraceLimit;

    if (stackTraceLimit > 0) {
        e.stackTraceLimit = stackTraceLimit + 1;
    }
    
    e.prepareStackTrace = (prepare: any, stack: any) => stack;
    // We remove stack at top since it's always this function.
    let stack = new e().stack.slice(1);
    e.prepareStackTrace = originPrepare;

    if (stackTraceLimit > 0) {
        e.stackTraceLimit = originLimit;
    }
    return stack;
}

/// <summary> Format stack trace. </summary>
export function formatStackTrace(trace: CallSite[]): string {
    let s = '';
    for (let site of trace) {
        s += 'at ' 
            + site.getFunctionName() 
            + '(' 
            + site.getFileName() 
            + ':' 
            + site.getLineNumber()
            + ':'
            + site.getColumnNumber()
            + ")\n";
    }
    return s;
}