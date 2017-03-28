import * as path from 'path';
import * as utils from './utils';
import * as objectModel from './object-model';
import * as app from './app';
import * as metrics from '@napajs/metrics'
import * as engine from './engine'

//////////////////////////////////////////////////////////////////////////////////
// Static classes that create or read definition objects.

const SCHEMA_DIR: string = path.resolve(__dirname, '../schema');  

/// <summary> Helper class to read TypeDefinition array from config. </summary>
export class ObjectTypeConfig {
    /// <summary> JSON schema used to validate conf. </summary>
    private static readonly OBJECT_TYPE_CONFIG_SCHEMA: utils.JsonSchema = 
        new utils.JsonSchema(path.resolve(SCHEMA_DIR, "object-type-config.schema.json"));

    /// <summary> Transforms from config object to definition. </summary>
    private static _transform: utils.Transform =
        new utils.SetDefaultValue({
            'override': false
        });

    /// <summary> Create TypeDefinition array from a JS object array that conform with schema.
    /// Throw exception if JS object array doesn't match schema.
    /// Schema: "../schema/object-type-config.schema.json"
    /// </summary>
    /// <param name="jsValue"> a JS object array to create TypeDefinition object. </param>
    /// <param name="validateSchema"> Whether validate schema, 
    /// this option is given due to request object already checked schema at request level. </param>
    /// <returns> A list of TypeDefinition objects. </returns>
    public static fromConfigObject(jsValue: any[], validateSchema: boolean = true): objectModel.TypeDefinition[] {
        if (validateSchema) {
            utils.ensureSchema(jsValue, this.OBJECT_TYPE_CONFIG_SCHEMA);
        }

        jsValue.forEach(obj => {
            this._transform.apply(obj);
        });
        return jsValue;
    }

    /// <summary> From a config file to create a array of TypeDefinition. </summary>
    /// <param name="objectTypeConfig"> TypeDefinition config file. </param>
    /// <returns> A list of TypeDefinition objects. </returns>
    public static fromConfig(objectTypeConfig: string): objectModel.TypeDefinition[] {
        return utils.appendMessageOnException(
            "Error found in object type definition file '" + objectTypeConfig + "'.",
            () => { return this.fromConfigObject(utils.readConfig(objectTypeConfig)); });
    }
}

/// <summary> Helper class to read ProviderDefinition array from config. </summary>
export class ObjectProviderConfig {
    /// <summary> JSON schema used to validate conf. </summary>
    private static readonly OBJECT_PROVIDER_CONFIG_SCHEMA: utils.JsonSchema = 
        new utils.JsonSchema(path.resolve(SCHEMA_DIR, "object-provider-config.schema.json"));

    /// <summary> Transform from config object to definition. </summary>s
    private static _transform: utils.Transform =
        new utils.SetDefaultValue({
            'override': false
        });
    
    /// <summary> Create ProviderDefinition array from a JS value that conform with schema.
    /// Throw exception if JS object array doesn't match schema.
    /// </summary>
    /// <param name="jsValue"> a JS value to create ProviderDefinition object. </param>
    /// <param name="validateSchema"> Whether validate schema, 
    /// this option is given due to request object already checked schema at request level. </param>
    /// <returns> A list of ProviderDefinition objects. </returns>
    public static fromConfigObject(jsValue: any[], validateSchema: boolean = true): objectModel.ProviderDefinition[]{
        if (validateSchema) {
            utils.ensureSchema(jsValue, this.OBJECT_PROVIDER_CONFIG_SCHEMA);
        }

        jsValue.forEach(obj => {
            this._transform.apply(obj);
        });
        return jsValue;
    }

    /// <summary> Create ProviderDefinition array from a configuration file. (.config or .json)
    /// Throw exception if JS object array doesn't match schema.
    /// Schema: "../schema/object-provider-config.schema.json"
    /// </summary>
    /// <param name="objectProviderConfig"> a JSON file in object provider definition schema. </param>
    /// <returns> A list of ProviderDefinition objects. </returns>
    public static fromConfig(objectProviderConfig: string): objectModel.ProviderDefinition[] {
        return utils.appendMessageOnException(
            "Error found in object provider definition file '" + objectProviderConfig + "'.",
            () => { return this.fromConfigObject(utils.readConfig(objectProviderConfig)); });
    }
}

/// <summary> Helper class to read NamedObjectDefinition array from config. </summary>
export class NamedObjectConfig {
    /// <summary> JSON schema used to validate conf. </summary>
    static readonly NAMED_OBJECT_CONFIG_SCHEMA: utils.JsonSchema = 
        new utils.JsonSchema(path.resolve(SCHEMA_DIR, "named-object-config.schema.json"));

    /// <summary> Transform object from JSON to object. </summary>
    private static _transform: utils.Transform =
        new utils.SetDefaultValue({
            'override': false,
            'private': false
        });

    /// <summary> Create NamedObjectDefinition array from a JS object array that conform with schema.
    /// Throw exception if JS object array doesn't match schema.
    /// Schema: "../schema/named-object-config.schema.json"
    /// </summary>
    /// <param name="jsValue"> a JS value array to create NamedObjectDefinition object. </param>
    /// <param name="validateSchema"> Whether validate schema, 
    /// this option is given due to request object already checked schema at request level. </param>
    /// <returns> A list of NamedObjectDefinition objects. </returns>
    public static fromConfigObject(jsValue: any[], validateSchema: boolean = true): objectModel.NamedObjectDefinition[]{
        if (validateSchema) {
            utils.ensureSchema(jsValue, this.NAMED_OBJECT_CONFIG_SCHEMA);
        }

        jsValue.forEach(obj => {
            this._transform.apply(obj);
        });
        return jsValue;
    }

    /// <summary> Create NamedObjectDefinition array from a configuration file. (.config or .json)
    /// Throw exception if configuration file parse failed or doesn't match schema.
    /// Schema: "../schema/named-object-config.schema.json"
    /// </summary>
    /// <param name="namedObjectConfigFile"> a JSON file in named object definition schema. </param>
    /// <returns> A list of NamedObjectDefinition objects. </returns>
    public static fromConfig(namedObjectConfigFile: string): objectModel.NamedObjectDefinition[] {
        return utils.appendMessageOnException(
            "Error found in named object definition file '" + namedObjectConfigFile + "'.",
            () => { return this.fromConfigObject(utils.readConfig(namedObjectConfigFile)); });
    }
}

/// <summary> Helper class to read MetricDefinition array from config. </summary>
export class MetricConfig {
    /// <summary> JSON schema used to validate config. </summary>
    private static readonly METRIC_CONFIG_SCHEMA: utils.JsonSchema 
        = new utils.JsonSchema(path.resolve(SCHEMA_DIR, "metric-config.schema.json"));

    /// <summary> Transform object from JSON to object. </summary>
    private static _transform: utils.Transform =
        new utils.SetDefaultValue( {
            'dimensionNames': []
        }).add(
        new utils.TransformPropertyValues({
            'type': (metricTypeName: string) => {
                let lowerCaseTypeName = metricTypeName.toLowerCase();
                switch (lowerCaseTypeName) {
                    case 'number': return metrics.MetricType.Number;
                    case 'rate': return metrics.MetricType.Rate;
                    case 'percentile': return metrics.MetricType.Percentile;
                    case 'latency': return metrics.MetricType.Percentile;
                    default: throw new Error("Invalid metric type: '" + metricTypeName + "'.");
                }
            }
        }));

    /// <summary> Create MetricDefinition array from a JS object array that conform with schema.
    /// Throw exception if JS object array doesn't match schema.
    /// Schema: "../schema/metric-config.schema.json"
    /// </summary>
    /// <param name="sectionName"> Section name used to create counters. </param>
    /// <param name="jsValue"> a JS value array to create MetricDefinition object. </param>
    /// <returns> A list of NamedObjectDefinition objects. </returns>
    public static fromConfigObject(sectionName: string, jsValue: any[]): app.MetricDefinition[] {
        utils.ensureSchema(jsValue, this.METRIC_CONFIG_SCHEMA);
        
        jsValue.forEach(obj => {
            this._transform.apply(obj);
            obj.sectionName = sectionName;
        });
        return <app.MetricDefinition[]>(jsValue);
    }

    /// <summary> Create MetricDefinition array from a configuration file. (.config or .JSON)
    /// Throw exception if JS object array doesn't match schema.
    /// Schema: "../schema/metric-config.schema.json"
    /// </summary>
    /// <param name="metricConfigFile"> a .config or .JSON file in metric definition schema. </param>
    /// <returns> A list of MetricDefinition objects. </returns>
    public static fromConfig(sectionName: string, metricConfigFile: string): app.MetricDefinition[] {
        return utils.appendMessageOnException(
            "Error found in metric definition file '" + metricConfigFile + "'.",
            () => { return this.fromConfigObject(sectionName, utils.readConfig(metricConfigFile)); });
    }
}

/// <summary> Helper class to read ApplicationSettings from config. </summary>
export class ApplicationConfig {
    /// <summary> JSON schema used to validate config. </summary>
    private static readonly APP_CONFIG_SCHEMA: utils.JsonSchema 
        = new utils.JsonSchema(path.resolve(SCHEMA_DIR, "application-config.schema.json"));

    /// <summary> Create ApplicationSettings from a JS object that conform with schema.
    /// Throw exception if JS object doesn't match schema.
    /// Schema: "../schema/application-config.schema.json"
    /// </summary>
    /// <param name="parentSettings"> Parent settings to inherit as default values. </param>
    /// <param name="jsValue"> a JS value to create ApplicationSettings object. </param>    
    /// <param name="basePath"> Base path used to resolve relative paths. </param>
    /// <returns> An ApplicationSettings object. </returns>
    public static fromConfigObject(
        parentSettings: app.Settings, 
        jsValue: any, 
        basePath: string): app.ApplicationSettings {

        utils.ensureSchema(jsValue, this.APP_CONFIG_SCHEMA);

        let appSettings: app.ApplicationSettings = {
            baseDir: basePath,
            id: jsValue.id,
            description: jsValue.description,
            allowPerRequestOverride: jsValue.allowPerRequestOverride,
            defaultExecutionStack: jsValue.defaultExecutionStack,
            objectContext: parentSettings.objectContext,
            metrics: []
        };

        // Optional: allowPerRequestOverride. 
        // Inherit engine settings if it's not provided from application.
        if (appSettings.allowPerRequestOverride == null) {
            appSettings.allowPerRequestOverride = parentSettings.allowPerRequestOverride;
        }

        // Optional: defaultExecutionStack. 
        // Inherit engine settings if it's not provided from application.
        if (appSettings.defaultExecutionStack == null) {
            appSettings.defaultExecutionStack = parentSettings.defaultExecutionStack;
        }

        // Required: 'objectTypes'
        let typeDefFiles: string[] = jsValue.objectTypes;
        let typeDefinitions: objectModel.TypeDefinition[] = [];
        let typeToFileName: { [typeName: string]: string } = {};
        for (let typeDefFile of typeDefFiles) {
            let typeDefs = ObjectTypeConfig.fromConfig(path.resolve(basePath, typeDefFile));
            for (let typeDefinition of typeDefs) {
                if (typeToFileName.hasOwnProperty(typeDefinition.typeName)
                    && !typeDefinition.override) {
                    throw new Error("Object type '"
                        + typeDefinition.typeName
                        + "' already exists in file '"
                        + typeToFileName[typeDefinition.typeName]
                        + "'. Did you forget to set property 'override' to true? ");
                }
                typeDefinitions.push(typeDefinition);
                typeToFileName[typeDefinition.typeName] = typeDefFile;
            }
        }

        // Optional: 'objectProviders'
        let providerDefFiles: string[] = jsValue.objectProviders;
        let providerDefinitions: objectModel.ProviderDefinition[] = [];
        let protocolToFileName: { [protocolName: string]: string } = {};
        if (providerDefFiles != null) {
            for (let providerDefFile of providerDefFiles) {
                let providerDefs = ObjectProviderConfig.fromConfig(path.resolve(basePath, providerDefFile));
                for (let providerDef of providerDefs) {
                    if (protocolToFileName.hasOwnProperty(providerDef.protocol)
                        && !providerDef.override) {
                        throw new Error("Object provider with protocol '"
                            + providerDef.protocol
                            + "' already exists in file '"
                            + protocolToFileName[providerDef.protocol]
                            + "' .Did you forget to set property 'override' to true? ");
                    }
                    providerDefinitions.push(providerDef);
                    protocolToFileName[providerDef.protocol] = providerDefFile;
                }
            }
        }

        // Required: 'namedObjects'
        let namedObjectDefFiles: string[] = jsValue.namedObjects;
        let namedObjectDefinitions: objectModel.NamedObjectDefinition[] = [];
        let nameToFileName: {[objectName: string]: string} = {};

        for (let namedObjectDefFile of namedObjectDefFiles) {
            let objectDefs = NamedObjectConfig.fromConfig(path.resolve(basePath, namedObjectDefFile));
            for (let objectDef of objectDefs) {
                if (nameToFileName.hasOwnProperty(objectDef.name)
                    && !objectDef.override) {
                    throw new Error("Named object'"
                        + objectDef.name
                        + "' already exists in file '"
                        + nameToFileName[objectDef.name]
                        + "'. Did you forget to set property 'override' to true? ");
                }
                namedObjectDefinitions.push(objectDef);
                nameToFileName[objectDef.name] = namedObjectDefFile;
            }
        }

        appSettings.objectContext = new objectModel.ScopedObjectContextDefinition(
            parentSettings.objectContext,
            typeDefinitions,
            providerDefinitions,
            namedObjectDefinitions,
            true                        // Enable depenency check.
            );      

        // Optional: 'metrics'
        let metricDefObject: any = jsValue.metrics;
        if (metricDefObject != null) {
            let sectionName = metricDefObject.sectionName;                    
            let metricDefFiles: string[] = metricDefObject.definition;
            let metricToFilename: { [metricName: string]: string } = {}
            
            metricDefFiles.forEach(metricDefFile => {
                let metricDefs = MetricConfig.fromConfig(
                    sectionName,
                    path.resolve(basePath, metricDefFile));

                metricDefs.forEach(metricDef => {
                    if (metricToFilename.hasOwnProperty(metricDef.name)) {
                        throw new Error("Metric '"
                            + metricDef.name
                            + "' already defined in file '"
                            + metricToFilename[metricDef.name]
                            + "'.");
                    }
                    appSettings.metrics.push(metricDef);
                    metricToFilename[metricDef.name] = metricDefFile;
                });
            });
        }
        return appSettings;
    }

    /// <summary> Create ApplicationSettings object from application config file (.config or .json)
    /// Throws exception if configuration file parse failed or doesn't match the schema.
    /// Schema: '../schema/application-config.schema.json'
    /// </summary>
    /// <param name="parentSettings"> Parent settings to inherit. </param>
    /// <param name="appConfigFile"> a JSON file in application config schema. </param>
    public static fromConfig(
        parentSettings: app.Settings, 
        appConfigFile: string): app.ApplicationSettings {

        return utils.appendMessageOnException(
            "Error found in application definition file '" + appConfigFile + "'.",
            () => { 
                return this.fromConfigObject(
                    parentSettings,
                    utils.readConfig(appConfigFile), path.dirname(appConfigFile)); 
            });
    }
}

/// <summary> Helper class to read EngineSettings from config. </summary>
export class EngineConfig {
    /// <summary> JSON schema used to validate config. </summary>
    private static readonly SETTINGS_SCHEMA: utils.JsonSchema 
        = new utils.JsonSchema(path.resolve(SCHEMA_DIR, "engine-config.schema.json"));

    /// <summary> Create EngineSettings from a JS object that conform with schema.
    /// Throw exception if JS object doesn't match schema.
    /// Schema: "../schema/engine-config.schema.json"
    /// </summary>
    /// <param name="jsValue"> a JS value to create EngineSettings object. </param>    
    /// <param name="basePath"> Base path used to resolve relative paths. </param>
    /// <returns> An EngineSettings object. </returns>
    public static fromConfigObject(jsValue: any, basePath: string): engine.EngineSettings {
         utils.ensureSchema(jsValue, this.SETTINGS_SCHEMA);
         
         let typeDefinitions: objectModel.TypeDefinition[] = [];
         if (jsValue.objectTypes != null) {
             for (let fileName of <string[]>(jsValue.objectTypes)) {
                let filePath = path.resolve(basePath, fileName);
                typeDefinitions = typeDefinitions.concat(ObjectTypeConfig.fromConfig(filePath));
             }
         }

         let providerDefinitions: objectModel.ProviderDefinition[] = [];
         if (jsValue.objectProviders != null) {
            for (let fileName of <string[]>(jsValue.objectProviders)) {
                let filePath = path.resolve(basePath, fileName);
                providerDefinitions = providerDefinitions.concat(ObjectProviderConfig.fromConfig(filePath));
             }
         }

         let namedObjectDefinitions: objectModel.NamedObjectDefinition[] = [];
         if (jsValue.namedObjects != null ){
            for (let fileName of <string[]>(jsValue.namedObjects)) {
                let filePath = path.resolve(basePath, fileName);
                namedObjectDefinitions = namedObjectDefinitions.concat(NamedObjectConfig.fromConfig(filePath));
             }
         }
         
         return {
            baseDir: basePath,
            allowPerRequestOverride: jsValue.allowPerRequestOverride,
            throwExceptionOnError: jsValue.throwExceptionOnError,
            defaultExecutionStack: jsValue.defaultExecutionStack,
            objectContext: new objectModel.ScopedObjectContextDefinition(
                null,
                typeDefinitions,
                providerDefinitions,
                namedObjectDefinitions,
                true)
            
         };
    }

    /// <summary> Create EngineSettings object from engine config file (.config or .json)
    /// Throws exception if configuration file parse failed or doesn't match the schema.
    /// Schema: '../schema/engine-config.schema.json'
    /// </summary>
    /// <param name="engineConfigFile"> a JSON file in engine config schema. </param>
    /// <returns> An EngineSettings object. </returns>
    public static fromConfig(engineConfigFile: string): engine.EngineSettings {
        return utils.appendMessageOnException(
            "Error found in vinyard setting file: '" +engineConfigFile + "'.",
            () => {
                return this.fromConfigObject(
                    utils.readConfig(engineConfigFile),
                    path.dirname(engineConfigFile));
            });
    }
}