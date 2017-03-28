import * as path from 'path';

import * as utils from './utils';
import { ObjectContext } from './object-context';

//////////////////////////////////////////////////////////////////////////
/// Interfaces and classes for URI based object retrieval.

/// <summary> Class that encapsulate parsing on URI.
/// A URI in Napa is defined in syntax: <protocol>:/<path>[?<param1>=<value1>[&<param2>=<value2>]*]
/// e.g. doc:/1E2B3C?env=os-prod-co3&type=js
/// TODO: replace this with 'url' module from Node.JS.
/// </summary>
export class Uri {

    /// <summary> Protocol of a URI. Case insensitive. 
    /// e.g. For URI 'doc:/1e2bcd3a?a=1&b=2', protocol is 'doc'.
    /// </summary>
    public protocol: string = null;

    /// <summary> Path of a URI. Case insensitive.
    /// e.g. For URI 'doc:/1e2bcd3a?a=1&b=2', path is '1e2bcd3a'.
    /// </summary>
    public path: string = null;

    /// <summary> Parameter map of a URI. Parameters are accessed by getParameter and setParameter methods.
    /// e.g, For URI 'doc:/1e2bcd3a?a=1&b=2', parameters are { 'a' : 1, 'b' : 2}
    /// </summary>
    private _parameters: { [key: string]: string } = {};

    /// <summary> Parse string input. Throws exception if parsing failed. </summary>
    /// <param name="input"> A URI string </param>
    /// <returns> A Uri object </returns>
    public static parse(input: string): Uri {
        let result = this.tryParse(input);
        if (!result.success) {
            throw new Error("Invalid URI string '" + input + "'");
        }
        return result.uri;
    }

    /// <summary> Try to parse string input. Returns results and uri. </summary>
    /// <param name="input"> A URI string </param>
    /// <returns> A result structue that contains success indicator and a created Uri object. Uri object will be null if success flag is false. </returns>
    public static tryParse(input: string): { success: boolean, uri: Uri } {
        // Syntax: <protocol>:/<path>[?<param1>=<value1>[&<param2>=<value2>]*]
        let start = 0;
        let end = input.indexOf(":/");
        if (end <= 0) {
            return { success: false, uri: null };
        }
        let uri = new Uri();
        uri.protocol = input.substring(start, end);

        // Find path.
        let path = "";
        start = end + 2;
        end = input.indexOf("?", start);
        if (end == -1) {
            // No parameters.
            uri.path = input.substring(start);
        }
        else {
            // Has parameters.
            uri.path = input.substring(start, end);
            let kvStrings = input.substring(end + 1).split('&');
            for (let i = 0; i < kvStrings.length; ++i) {
                let cols = kvStrings[i].split('=');
                if (cols.length != 2) {
                    return { success: false, uri: null };
                }
                uri.setParameter(cols[0], cols[1]);
            }
        }
        return { success: true, uri: uri };
    }

    /// <summary> Retrieve parameter value by name </summary>
    /// <param name="name"> Case insensitive name. </param>
    /// <returns> Parameter value as string. </returns>
    public getParameter(name: string): string {
        return this._parameters[name.toLowerCase()];
    }

    /// <summary> Set paramter value by name. </summary>
    /// <param name="name"> Case-insensitive name. </param>
    /// <param name="value"> Parameter value. Suggested interpretion in case-insensitive manner. </param>
    public setParameter(name: string, value: string) {
        this._parameters[name.toLowerCase()] = value;
    }

    /// <summary> Tell if an input string is a URI or not. </summary>
    /// <param name="input"> Input string. </param>
    /// <returns> True if input is a URI, otherwise False. </returns>
    public static isUri(input: string): boolean {
        return Uri.tryParse(input).success;
    }
}

/// <summary> Function interface that takes a Uri as parameter and produce a JS value </summary>
export type ObjectLoader = (uri: Uri | Uri[], context?: ObjectContext) => any;

/// <summary> Interface for provider that retrieve object via a URI </summary>
export interface ObjectProvider {
    /// <summary> Provide any JS value from a URI. </summary>
    /// <param name="uri"> a URI object or array of URIs. </param>
    /// <param name="context"> Object context if needed to create sub-objects. </param>
    /// <returns> Any JS value. </returns>
    /// <remarks>
    /// On implementation, you can check whether input is array or not as Array.isArray(input).
    /// </remarks>
    provide(uri: Uri | Uri[], context?: ObjectContext): any;

    /// <summary> Check if current provider support a protocol name.</summary>
    /// <param name="protocol"> Case insensitive protocol name. </param>
    /// <returns> True if protocol is supported, otherwise false. </param>
    supports(protocol: string): boolean;
}

/// <summary> Object provider definition to register a URI based object provider in Napa. </summary>
export interface ProviderDefinition {
    protocol: string;
    description?: string;
    moduleName: string;
    functionName: string;
    override?: boolean;
    exampleUri?: string[];
}

/// <summary> An implementation of ObjectProvider via protocol based registry. </summary>
export class ProviderRegistry implements ObjectProvider {

    /// <summary> Map of protocol (lower-case) to object loader. </summary>
    private _protocolToLoaderMap: Map<string, ObjectLoader> = new Map<string, ObjectLoader>();

    /// <summary> Provide any JS value from a URI object. 
    /// <param name="uri"> a URI object or URI array </param>
    /// <returns> Any JS value. </returns>
    public provide(uri: Uri | Uri[], context?: ObjectContext): any {
        let protocol: string;
        if (Array.isArray(uri)) {
            if ((<Uri[]>uri).length == 0) {
                return null;
            }
            protocol = (<Uri[]>uri)[0].protocol;
            for (let item of uri) {
                if (item.protocol !== protocol) {
                    throw new Error("Protocol must the the same with an array of Uris when calling ObjectContext.create.");
                }
            }
        }
        else {
            protocol = (<Uri>uri).protocol;
        }
        let lowerCaseProtocol = protocol.toLowerCase();
        if (this.supports(lowerCaseProtocol)) {
            return this._protocolToLoaderMap.get(lowerCaseProtocol)(uri, context);
        }
        throw new Error("Unsupported protocol '" + protocol + "'.");
    }

    /// <summary> Register an object provider with a protocol. Later call to this method on the same protocol will override the provider of former call.</summary>
    /// <param name="type"> Case insensitive protocol name.</param>
    /// <param name="creator"> An object provider.</param>
    public register(protocol: string, loader: ObjectLoader): void {
        this._protocolToLoaderMap.set(protocol.toLowerCase(), loader);
    }

    /// <summary> Check if current provider support a protocol name.</summary>
    /// <param name="protocol"> Case insensitive protocol name. </param>
    /// <returns> True if protocol is supported, otherwise false. </param>
    public supports(protocol: string): boolean {
        return this._protocolToLoaderMap.has(protocol.toLowerCase());
    }

    /// <summary> Created ProviderRegistry from a collection of ProviderDefinition objects. </summary>
    /// <param name="providerDefCollection"> Collection of ProviderDefinition objects. </summary>
    /// <param name="baseDir"> Base directory name according to which module name will be resolved.</param>
    /// <returns> A ProviderRegistry object. </returns>
    public static fromDefinition(providerDefCollection: ProviderDefinition[], baseDir: string): ProviderRegistry {
        let registry = new ProviderRegistry();
        if (providerDefCollection != null) {
            for (let def of providerDefCollection) {
                let moduleName = def.moduleName;
                if (!path.isAbsolute(def.moduleName)) {
                    moduleName = path.resolve(baseDir, moduleName);
                }
                let creator = utils.loadFunction(moduleName, def.functionName);
                registry.register(def.protocol, creator);
            }
        }
        return registry;
    }
}
