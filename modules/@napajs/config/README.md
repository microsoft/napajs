# @napajs/config

A facade module to access configuration objects. There are 3 aspects of configuration reading:
1) File locator

## API

```ts
/// <summary> Function interface for file locator. </summary>
export interface FileLocator {
    (configName: string): string
}

/// <summary> Function interface for config parser. </summary>
export interface Parser {
    parse(fileName: string): any;
}

/// <summary> Class for config object schema. </summary>
export declare class Schema {
    /// <summary> Construct a schema object. </summary>
    /// <param name="jsonSchemaFile"> JSON schema describe the final object read from file. This schema is not coupled with config format.</param>
    constructor(jsonSchemaFile: string);

    /// <summary> Validate an object against the schema and fill default values. </summary>
    /// <param name="jsValue"> A Javascript value. </param>
    validate(jsValue: any): boolean;
}

/// <summary> Set a custom parser for a file extension. 
/// If multiple calls are made to setParser on a file extension, the last one will be used.
/// </summary>
/// <param name="fileExtension"> File extension name without '.' . </param>
/// <param name="parser"> A parser function for the file extension . </param>
export function setParser(fileExtension: string, parser: Parser): void;

/// <summary> Set a file locator for resolving config file. 
/// If multiple calls are made to setLocator, the last one will be used.
/// </summary>
/// <param name="locator"> A file locator function. </param>
export function setLocator(locator: FileLocator): void;

/// <summary> Read a config object from a config identifier. Validate against schema and fill default values if provided. </summary>
/// <param name="configId"> An identifier of config that will be passed to FileLocator to resolve into a file path. </param>
/// <param name="schema"> (optional) schema object to validate config and fill default values. </schema>
export function read(configId:string, schema?: Schema): any;

/// <summary> Namespace for default facilities. </summary>
namespace defaults {
    /// <summary> Default file locator, which resolves the configName as file name in working directory. </summary>
    export var FileLocator;

    /// <summary> Default Ini parser. </summary>
    export var IniParser;

    /// <summary> Default Xml parser. </summary>
    export var XmlParser;

    /// <summary> Default Json parser, which is JSON.parse. </summary>
    export var JsonParser;
}

```
## Usage

Set up custom locator and parser.

```ts
import * as config from '@napajs/config';
import * as path from 'path';
import * as fs from 'fs';

config.setLocator((configName: string): string => {
    let extensions = ['json', 'xml', 'ini'];
    for (let ext of extensions) {
        let filePath = path.resolve(process.cwd(), `${configName}.${ext}`);
        if (fs.existsSync(filePath)) {
            return filePath;
        }
    }
    throw new Error("File doesn't exist for config: " + configName);
});

config.setParser('json', (fileName: string): any => {
    return JSON.parse(fileName);
});

```
Parse config.
```ts

// Read test.json, or test.xml or test.ini if exists.
// test.json:
// { "threshold": 1 }
// test-schema.json has default value for 'enabled': true.
let schema = new Schema('test-schema.json')
let testConfig = config.read('test', schema);

// returns {"threshold": 1, "enabled": true }
console.log(testConfig);

```