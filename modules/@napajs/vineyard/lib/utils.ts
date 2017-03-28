//////////////////////////////////////////////////////////////////
// This file defines utility classes used in Napa
//

import fs = require('fs');
// TODO: replace bundle with node module dependencies once 'url' and 'events' are introduced in Napa.JS.
var Ajv = require('./ajv-bundle');
var xml2js = require('./xml2js-bundle');

/// TODO: move config reading to @napajs/config module.
/// <summary> Class that wraps JSON schema validation. </summary>
export class JsonSchema {
    private _fileName: string;

    private _validator: any;

    /// <summary> Static AJV engine to create pre-compiled validator </summary>
    /// This path is to be compatible with V8 isolates/
    private static _ajv = Ajv();

    /// <summary> Validate a JS value against current schema. </summary>
    /// <param name="jsValue"> a JS value </param>
    /// <returns> True if validation succeeded, otherwise false. </returns>
    public validate(jsValue: any): boolean {
        return this._validator(jsValue);
    }

    /// <summary> Get schema file name. </summary>
    /// <returns> Schema file name. </returns>
    public getFileName(): string {
        return this._fileName;
    }

    /// <summary> Get errors from previous validation. </summary>
    /// <returns> Errors from previous validation. </returns>
    public getErrors(): string {
        return this._validator.errors;
    }

    /// <summary> Constructor </summary>
    /// <param name="fileName"> JSON schema file name.</param>
    public constructor(fileName: string) {
        this._fileName = fileName;
        var schemaObject = parseJsonFile(fileName);
        /// This path is to be compatible with V8 isolates/
        this._validator = JsonSchema._ajv.compile(schemaObject);
    }
}

/// <summary> Ensure a JS value match a JSON schema. Throws exception if it doesn't match. </summary>
/// <param name="jsValue"> Any JS value type. </param>
/// <param name="jsonSchema"> JSON schema object. </param>
export function ensureSchema(jsValue: any, jsonSchema: JsonSchema): void {
    if (!jsonSchema.validate(jsValue)) {
        throw new Error(
            "Object '"
            // + JSON.stringify(jsValue)
            + "' doesn't match schema '"
            + jsonSchema.getFileName() + "':"
            + JSON.stringify(jsonSchema.getErrors()));
    }
}

/// <summary> Parse JSON string into JS value. Validate with json schema if present. </summary>
/// <param name="jsonString"> JSON string. </param>
/// <param name="jsonSchema"> JSON schema object. Optional </param>
/// <param name="allowComments"> Whether allow comments in JSON. 
/// REMARKS: PLEASE NOT TO ENABLE THIS DURING QUERY PROCESSING, WHICH IS REALLY SLOW < /param>
/// <returns> JS value. Throws exception if parse failed or schema validation failed. </returns>
export function parseJsonString(jsonString: string,
    jsonSchema: JsonSchema = undefined,
    allowComments: boolean = false): any {

    /// This path is to be compatible with V8 isolates/
    if (allowComments) {
        var stripJsonComments = require('strip-json-comments');
        jsonString = stripJsonComments(jsonString);
    }

    try {
        var jsValue = JSON.parse(jsonString);
    }
    catch (error) {
        throw new Error("Failed to parse JSON ':" + error.toString());
    }

    if (jsonSchema != null) {
        ensureSchema(jsValue, jsonSchema);
    }
    return jsValue;
}

/// <summary> Parse JSON file. 
/// Throw exception if parse failed or JSON schema validation failed.
/// </summary>
/// <param name="jsonFile"> JSON file to parse. </param>
/// <param name="jsonSchema"> JSON schema object. </param>
/// <param name="allowComments"> Whether allow comments in JSON. </param>
/// <returns> JS value parsed from JSON file. </returns>
export function parseJsonFile(jsonFile: string,
    jsonSchema: JsonSchema = undefined,
    allowComments: boolean = false): any {
    return appendMessageOnException(
        ".Error file name: '" + jsonFile + "'.",
        () => { return parseJsonString(readJsonString(jsonFile), jsonSchema, allowComments); });
}

/// <summary> Read JSON string from file. </summary>
export function readJsonString(jsonFile: string): string {
    return fs.readFileSync(jsonFile, 'utf8').replace(/^\uFEFF/, '');
}

/// <summary> Parse JS values from an XML file. Please see summary of XmlObjectReader class for details. </summary>
/// <param name="xmlFilePath"> XML file path. </param>
/// <param name="jsonSchema"> JSON schema to validate object. optional. </param>
/// <returns> JS values if parsed successfully. Throw exception if failed. </returns>
export function parseXmlFile(xmlFilePath: string, jsonSchema?: JsonSchema): any {
    return XmlObjectReader.default.readFile(xmlFilePath, jsonSchema);
}

/// <summary> Parse JS values from an XML string. Please see summary of XmlObjectReader class for details. </summary>
/// <param name="xmlString"> XML string. </param>
/// <param name="jsonSchema"> JSON schema to validate object. optional. </param>
/// <returns> JS values if parsed successfully. Throw exception if failed. </returns>
export function parseXmlString(xmlString: string, jsonSchema?: JsonSchema): any {
    return XmlObjectReader.default.read(xmlString, jsonSchema);
}

/// <summary> Read JS value from a config file with extension '.config' OR '.json' in XML format.
/// When file path ends with '.config', it will first try to read object from '.config', 
/// if file '.config' is not presented, '.json' will be checked.
/// When file path ends with '.json', only '.json' will be checked.
///
/// This method is introduced to make Napa code transparent to the format of configuration file,
/// We can use Autopilot flattened XML file (with enables bed-specific configuration) or plain JSON file.
/// </summary>
/// <param name='filePath'> File path with ".config" or ".json" extension </param>
/// <returns> JS values if parsed successfully. Throw exception if failed. </returns>
export function readConfig(filePath: string, jsonSchema?: JsonSchema): any {
    var extensionStart = filePath.lastIndexOf('.');
    var extension = filePath.substring(extensionStart).toLowerCase();

    var checkForConfigExtFirst = false;
    if (extension == ".config") {
        checkForConfigExtFirst = true;
    }
    else if (extension != '.json') {
        throw new Error("readConfig only support '.config' and '.json' as extension. filePath='"
            + filePath + "'.");
    }

    if (checkForConfigExtFirst) {
        // Then check XML format .config file.
        var xmlConfigFile = filePath;
        if (fs.existsSync(xmlConfigFile)) {
            // TODO: dump converted JSON for debug purpose.
            return parseXmlFile(xmlConfigFile, jsonSchema);
        }
    }
    // Check JSON format .json file if XML format is not present.
    var jsonFile = filePath.substring(0, extensionStart) + ".json"
    
    // We allow comments in JSON configuration files.
    return parseJsonFile(jsonFile, jsonSchema, true);
}

/// <summary> Interface for JS value transformation. </summary>
export interface Transform {
    apply(jsValue: any): any;
};

export class ChainableTransform implements Transform {
    protected _next: ChainableTransform = null;
    public add(next: ChainableTransform): ChainableTransform { 
        var node: ChainableTransform = this;
        while (node._next != null) {
            node = node._next;
        }
        node._next = next;
        return this;
    }

    public apply(jsValue: any): any {
        var transformedValue = this.transform (jsValue);
        if (this._next != null) {
            return this._next.apply(transformedValue);
        }
        return transformedValue;
    }

    protected transform(jsValue: any): any {
        throw new Error("Not implemented");
    }
};

/// <summary> Rename properties of a JS object </summary>
export class RenameProperties extends ChainableTransform {
    private _nameMap: { [oldName: string]: string };

    /// <summary> Constructor </summary>
    /// <param name="nameMap"> Old name to new name mapping. </param>
    public constructor(nameMap: { [oldName: string]: string }) {
        super();
        this._nameMap = nameMap;
    }

    /// <summary> Do transformation by rename properties. </summary>
    /// <param name="jsObject"> container JS object to rename properties. </param>
    public transform(jsObject: {[propertyName:string]: any}): any {
        var oldNames: string[] = Object.keys(this._nameMap);
        oldNames.forEach(oldName => {
            jsObject[this._nameMap[oldName]] = jsObject[oldName];
            delete jsObject[oldName];
        });
        return jsObject;
    }
} 

/// <summary> Set default value for properties that are undefined or null </summary>
export class SetDefaultValue extends ChainableTransform {
    private _defaultValueMap: { [propertyName: string]: any };

    /// <summary> Constructor </summary>
    /// <param name="nameMap"> Property name to default value map. </param>
    public constructor(defaultValueMap: { [propertyName: string]: any }) {
        super();
        this._defaultValueMap = defaultValueMap;
    }

    /// <summary> Do transformation by set default values for fields that does't appear in object. </summary>
    /// <param name="jsObject"> JS object </param>
    public transform(jsObject: {[propertyName:string]: any}): any {
        var propertyNames: string[] = Object.keys(this._defaultValueMap);
        propertyNames.forEach(propertyName => {
            if (!jsObject.hasOwnProperty(propertyName)) {
                jsObject[propertyName] = this._defaultValueMap[propertyName];
            }
        });
        return jsObject;
    }
}

/// <summary> Interface for value transform function </summary>
export interface ValueTransform {
    (input: any): any
}

/// <summary> Value transfor of a JS value to another </summary>
export class TransformPropertyValues extends ChainableTransform {
    private _propertyNameToTransformMap: { [propertyName: string]: ValueTransform };

    /// <summary> Constructor </summary>
    /// <param name="nameMap"> Property name to value transform function. </param>
    public constructor(propertyNameToTransformMap: { [propertyName: string]: ValueTransform }) {
        super();
        this._propertyNameToTransformMap = propertyNameToTransformMap;
    }

    /// <summary> Do transformation by transforming values on properties. </summary>
    /// <param name="jsObject"> container JS object. </param>
    public transform(jsObject: { [propertyName: string]: any }): any {
        var oldNames: string[] = Object.keys(this._propertyNameToTransformMap);
        oldNames.forEach((propertyName: string) => {
            jsObject[propertyName] = this._propertyNameToTransformMap[propertyName](jsObject[propertyName]);
        });
        return jsObject;
    }
} 

/// <summary> An operation on a singular node that produce a output 
/// A singular node is a JS node which is a object like { } or an 1-element array. [{ //...}]
/// </summary>
interface SingularNodeOperation {
    (node: any): any;
}

/// <summary> Result from a singular node operation. </summary>
/// operationPerformed indicates if a target is a singular node or not.
/// result is the returned value from the operation. 
class SingularNodeOperationResult {
    public constructor(public operationPerformed: boolean, public result?: any) {
    }
}

/// <summary> A JS Object reader from XML file or string </summary>
/// We don't support arbitrary XML compositions from the input. Instead, we carefully
/// supported a sub-set of XML composition patterns that can translate to any JS object.
/// 
/// 3 types of XML composition patterns are supported that map to 3 types of JS objects.
/// 
/// 1. Property: Used for translating to a JS property.
///    <property1>value</property1>                 
///
/// Converted JS form would be: "property1": "value".
/// If the XML element is the root element, converted JS would simply be: "value".
/// 
/// Empty property is not valid semantically. 
/// <property1/> will be translated as an empty object as 'property1': {}.
///
/// 2. List: Used for translating to a JS list. 
/// "item" is used as the default XML element name for list items, which can contain either raw content or nested nodes.
///    <list1>
///       <item>value1</item>
///       <item>value2</item>
///    </list1>
///
/// We can use different XML element name by specifying 'itemElement' attributes at the list element.
///    <list1 itemElement='customizedItem'>
///       <customizedItem>value1</customizedItem>
///       <customizedItem>value2</customizedItem>
///    </list1>
///
/// Both examples above are converted to JS form like: "list1": ["value1", "value2"]
/// If list is root element in the XML, the output will be ["value1", "value2"].
///
/// Empty list will be represented as <list1 type='list'/>, which will be translated to "list1": [].
/// Without specifying 'type' attribute, <list1/> will be translated to "list1": {}.
///
/// 3. Object: Used for translating to a JS object.
/// Object node can have nested properties and lists. 
///    <object1>                                     
///      <property1>value</property1>
///      <list1>
///         <item>value1</item>
///         <item>true</item>
///         <item>2</item>
///         <item type="string">false</item>
///         <item type="string">3</item>
///      </list1>
///    </object1>
///
/// Converted JS form would be: "object1" : { "property1": "value", "list1": ["value1", true, 2, "false", "3"] }
/// If 'object1' is the root element, the JS form would be { "property1": "value", "list1": ["value1", true, 2, "false", "3"]}
/// 
/// Empty object will be reprsented as <object1/>, which will be translated to "object1": {}.
///
/// No attribute could be used in all XML element with 3 exceptions:
/// 1. attribute 'type'='string' | 'bool' | 'number' can be used to explicitly specify value type despite its value content.
/// 2. attribute 'type'="list' can be used to sepcify empty XML element to hint a empty list.
/// 3. attribute 'itemElement' can be used to speicfy customized XML element name for list items other than 'item'.
class XmlObjectReader {

    /// <summary> xml2js parser. </summary>
    private _parser: any;

    /// <summary> XML builder when we want to restore xml2js output to XML for error reporting. </summary>
    private _builder: any;

    /// <sumary> Default instance. </summary>
    private static _defaultInstance: XmlObjectReader = new XmlObjectReader();

    /// <summary> Return a default instance of XmlObjectReader </summary>
    public static get default(): XmlObjectReader {
        return this._defaultInstance;
    }

    public constructor() {
        this._parser = new xml2js.Parser({
            explicitRoot: false,
            explicitCharkey: true,
            preserveChildrenOrder: true,
        });

        this._builder = new xml2js.Builder({
            headless: true,
        });
    }

    /// <summary> Read a JS object from a XML file. </summary>
    /// <param name="xmlFile"> an XML file path. </param>
    /// <param name="jsonSchema"> JSON schema to validate object. optional. </param>
    /// <returns> JS object. </returns>
    public readFile(xmlFile: string, jsonSchema?: JsonSchema): any {
        var xmlString = fs.readFileSync(xmlFile, 'utf8');
        return this.read(xmlString, jsonSchema);
    }

    /// <summary> Read a JS object from a XML string. </summary>
    /// <param name="xmlString"> An XML string. </param>
    /// <param name="jsonSchema"> JSON schema to validate object. optional. </param>
    /// <returns> JS object. </returns>
    public read(xmlString: string, jsonSchema?: JsonSchema): any {
        var output: any = null;
        this._parser.parseString(xmlString, (error: any, result:any) => {
            output = this._transformNode(result);
        });
        if (jsonSchema) {
            ensureSchema(output, jsonSchema);
        }
        return output;
    }

    /// <summary> Transform a JS node after xml2js </summary>
    /// <param name="node"> Input node </param>
    /// <returns> Output JS node. </returns>
    private _transformNode(node: any): any {
        if (this._isProperty(node)) {
            return this._transformProperty(node);
        }
        else if (this._isList(node)) {
            return this._transformList(node);
        }
        else if (this._isObject(node)) {
            return this._transformObject(node);
        }
        else {
            throw new Error("Unsupported JSON pattern converted from XML composition:"
                + this._builder.buildObject(node));
        }
    }

    /// <summary> Check if a node is transformed from XML element has a specific attribute</summary>
    /// <param name="node"> Input node </param>
    /// <param name="attributeName"> Attribute name </param>
    /// <returns> True if has this attribute, otherwise false. </returns>
    private _hasXmlAttribute(node: any, attributeName: string): boolean {
        var ret = this._tryOperateOnSingularNode(node, (input) => {
            return input.hasOwnProperty("$") && input["$"].hasOwnProperty(attributeName);
        });
        return ret.operationPerformed && ret.result;
    }

    /// <summary> Get the value of XML attribute name from a node. </summary>
    /// <param name="node"> Input node </param>
    /// <param name="attributeName"> Attribute name </param>
    /// <returns> The value of the XML attribute. Throws exception if no such XML attribute. </returns>
    private _getXmlAttribute(node: any, attributeName: string): any {
        if (!this._hasXmlAttribute(node, attributeName)) {
            throw new Error("XML attribute '" + attributeName + "' doesn't exist in object: " + JSON.stringify(node));
        }
        return this._operateOnSingularNode(node, (input) => {
            return input["$"][attributeName];
        });
    }

    /// <summary> Perform operation on a singular node (a object or an 1-element array). </summary>
    /// <param name="node"> Input node </param>
    /// <param name="operation"> Operation </param>
    /// <returns> Return value from the operation. Exception will be thrown if the node is not a singular node. </returns>
    private _operateOnSingularNode(node: any, operation: SingularNodeOperation): any {
        var input = node;
        if (Array.isArray(node)) {
            if (node.length != 1) {
                throw new Error("Cannot perform singular node operation on an array which has more than one element."
                    + JSON.stringify(node));
            }
            input = node[0];
        }
        return operation(input);
    }

    /// <summary> Try perform operation on a singular node (a object or an 1-element array). </summary>
    /// <param name="node"> Input node </param>
    /// <param name="operation"> Operation </param>
    /// <returns> Return SingularNodeOperationResult. No exception will be thrown if node is not a singular node. </returns>
    private _tryOperateOnSingularNode(node: any, operation: SingularNodeOperation): SingularNodeOperationResult {
        var input = node;
        if (Array.isArray(node)) {
            if (node.length != 1) {
                return new SingularNodeOperationResult(false);
            }
            input = node[0];
        }
        return new SingularNodeOperationResult(true,
            operation(input));
    }

    // Property XML element is mapped into JS object by xml2js in 2 ways.
    // 1. If the element is the only element with the name under its container, like 
    //   <container>
    //      <name>value</name>
    //   </container>
    // the <name> element will be transformed to a property of its container:
    //   "name" : [{ "_": "value" }]
    // 
    // 2. If there are multiple elements using the same name under its container, like:
    //   <container>
    //       <name>value1 </name>
    //       <name>value2 </name >
    //   </container>
    // It will be transformed to a property of its container like:
    //   "name": [
    //      { "_": "value1" },
    //      { "_": "value2" },
    //   ]
    //
    // Here we check if a jsNode is a leaf XML element.
    private _isProperty(node: any): boolean {
        var ret = this._tryOperateOnSingularNode(node, (singleNode) => {
            // Empty object container.
            if (singleNode === "") {
                return false;
            }
            // Has content like <name>value</name>
            return singleNode.hasOwnProperty("_");
        });
        return ret.operationPerformed && ret.result;
    }

    // For property, by default string will be the value type. unless specified with 'type' property. 
    // <elem>true</elem> will be transform to: "elem": true
    // <elem type="string">true</elem> will be transform to: "elem": "true".
    // <elem>1</elem> will be transform to: "elem": 1.
    // <elem type="string">1</elem> will be transform to: "elem": "1".
    private _transformProperty(node: any): any {
        // NOTE: till here _isLeaf already performed on node.
        return this._operateOnSingularNode(node, (singleNode) => {
            var value: string = singleNode["_"];
            
            // If 'type' property is specified.
            var valueType: string = undefined;
            if (this._hasXmlAttribute(singleNode, 'type')) {
                valueType  = this._getXmlAttribute(singleNode, 'type').toLowerCase();
            }

            // Short-circuit explict string type.
            if (valueType === 'string') {
                return value;
            }

            // Auto detect bool. 
            if (value === 'true' || value === 'false') {
                return value === 'true';
            }
            else if (valueType === 'bool') {
                throw new Error("Invalid bool value: " + value);
            }

            // Auto detect number.
            var numValue = Number(value);
            if (!isNaN(numValue)) {
                return numValue;
            }
            else if (valueType === 'number') {
                throw new Error("Invalid number value: " + value);
            }
            return value;
        });
    }

    // List node is mapped from XML to JS in a way of this pattern:
    // <list>
    //   <!-- item is a leaf type -->
    //   <item>value1</item>
    //   <!-- item is a container type -->
    //   <item><property2>value2</property2></item> 
    // </list>
    // It will be transformed to JS object like:
    //   "list" : [
    //     { "item" [
    //          { "_": "value1"},
    //          { "property2" : [{ "_": value2 }] },
    //       ]}]
    //
    // Empty list <list type='list'/> will be transformed to
    //   "list" : [{"$": { "type": "list" }}] 
    private _isList(node: any): boolean {
        var ret = this._tryOperateOnSingularNode(node, (input) => {
            // Check for non-empty list.
            var props = Object.getOwnPropertyNames(input);
            var itemElementName = 'item';
            var expectedProps = 1;
            if (this._hasXmlAttribute(input, 'itemElement')) {
                itemElementName = this._getXmlAttribute(input, 'itemElement');
                ++expectedProps;
            }
            if (input.hasOwnProperty(itemElementName) && props.length == expectedProps) {
                return true;
            }
            // Check for empty list.
            if (input.hasOwnProperty('$') && props.length == 1) {
                var nodeType = input['$']['type'];
                if (nodeType != 'list') {
                    throw new Error("Only 'type' attribute is supported for empty XML element for denoting it is a list.");
                }
                return true;
            }
            return false;
        });
        return ret.operationPerformed && ret.result;
    }

    private _transformList(node: any): any[] {
        return this._operateOnSingularNode(node, (input) => {
            // Empty list.
            if (this._hasXmlAttribute(input, 'type')) {
                return [];
            }
            // List with customize item element.
            var itemElementName = 'item'
            if (this._hasXmlAttribute(input, 'itemElement')) {
                itemElementName = this._getXmlAttribute(input, 'itemElement');
            }

            var list: any[] = []
            input[itemElementName].forEach((item: any) => {
                list.push(this._transformNode(item));
            });
            return list;
        });
    }

    private _isObject(node: any): boolean {
        var ret = this._tryOperateOnSingularNode(node, (input) => {
            // Object cannot have content like leaf node.
            if (input.hasOwnProperty('_')) {
                return false;
            }
            var props = Object.getOwnPropertyNames(input);
            if (input.hasOwnProperty('$') && props.length > 1) {
                throw new Error("Napa doesn't support XML element with both attributes and sub-elements, except for attribute 'itemElement' for list. Element with issues:\n"
                    + this._builder.buildObject(node));
            }
            return true;
        });

        return ret.operationPerformed && ret.result;
    }

    private _transformObject(node: any): any {
        return this._operateOnSingularNode(node, (input) => {
            if (node == "") {
                /// Empty object.
                return {};
            }
            var output: any = {};
            var props = Object.getOwnPropertyNames(input);
            props.forEach((prop) => {
                output[prop] = this._transformNode(input[prop]);
            });
            return output;
        });
    }
}

/// <summary> Include file name in exception when thrown.
export function appendMessageOnException(message: string, fun: Function) : any {
    try {
        return fun();
    }
    catch (error) {
        error.message += message;
        throw error;
    }
}

/// <summary> Make a return value as a resolved Promise or return if it is already a Promise. </summary>
export function makePromiseIfNotAlready(returnValue: any): Promise<any> {
    if (returnValue != null 
        && typeof returnValue === 'object'
        && typeof returnValue['then'] === 'function') {
        return returnValue;
    }
    return Promise.resolve(returnValue);
}

/// <summary> Load a function object given a name from a module. </summary>
export function loadFunction(moduleName: string, functionName: string) {
    let module = require(moduleName);
    if (module == null) {
        throw new Error("Cannot load module '" + moduleName + "'.");
    }
    
    let func = module;
    for (let token of functionName.split(".")) {
        if (token.length == 0) {
            continue;
        }
        func = func[token];
        if (func == null) {
            throw new Error("Cannot load function '" 
                + functionName 
                + "' in module '" 
                + moduleName 
                + "'. Symbol '"
                + token
                + "' doesn't exist.");
        }
    }
    return func;
}