# napa/vanilla/node_modules: Napa module for both Node.JS and Napa container.

'napa' module is introduced to access napa capabilities from Node.JS and Napa containers.

## **Introduction**
1. napa.runtime is the sub-module for creating Napa containers and execute JavaScript in these containers, which is available only in Node.JS.
2. napa.runtime is the sub-module for hosting Napa applications, which is available for both Node.JS and Napa containers.

## **Usage**
### **Scenario 1: run CPU-intensive tasks in parallel.**

Node.JS cannot run CPU-intensive tasks in parallel, as it has only one thread. 
napa.runtime enables you to run multiple CPU-intensive tasks in parallel within a single instance of Node.JS.   

heavy-duty-task.js
```
var napa = require('napa');

// Create a Napa container with 8 threads, each thread with heap 128MB.
var container = napa.runtime.createContainer(
    {
        numberOfThreads: 8,
        heapSizeInMB: 128
    }
);

// Load entry JavaScript file before execution.
container.load('user-code-entry.js');
var args = [1, 'hello world', true];

// computationHeavyTask is a function defined in user-code-entry.js.
// args is an array of serializable objects that matches computationHeavyTask signature.  
container.run('computationHeavyTask', args, (status, response) => {
    if (status.code === napa.runtime.ResultCode.success) {
        console.log(JSON.parse(response));
    }
    else {
        console.error(status.errorMessage);
    }
});

```
### **Scenario 2: serve non-CPU-intensive applications.**

Node.JS provides a rich set of tools for building applications, while the logic is non-CPU-intensive (like IO operation, db lookup, web-site, etc.). 
What value could napa.app bring for non-CPU-intensive applications? A short answer is for dynamic object/code override at per-request level, and a set of framework capabilities (like object management, etc.). 

light-duty-app-server.js
```
var napa = require('napa');
napa.app.register('example-app');

// service method to invoke napa to process request.  
function handleRequest(jsonRequest) {
    return napa.app.process(jsonRequest);
});
```

### **Scenario 3: serve CPU-intensive applications.**

With the combination of napa.runtime and napa.app, you can serve CPU-intensive applications within a single Node.JS instance, with per-request override.

heavy-duty-app-server.js
```
var napa = require('napa');

// Create a Napa container with 8 threads, each thread with heap 128MB.
var container = napa.runtime.createContainer(
    {
        numberOfThreads: 8,
        heapSizeInMB: 128
    }
);

// Load entry JavaScript file before execution.
container.load('app-set.js');

// computationHeavyTask is a function defined in user-code-entry.js.
// args is an array of serializable objects that matches computationHeavyTask signature.
function handleRequest(jsonRequest) {  
    container.run('handleRequest', [jsonRequest], (status, response) => {
        if (status.code === napa.runtime.ResultCode.success) {
            return JSON.parse(response);
        }
        else {
            console.error(status.errorMessage);
            return null;
        }
    });
}

```

app-set.js
```
var napa = require('napa');
napa.app.register('bing');
napa.app.register('malta');
handleRequest = napa.app.process;
```

**TODO: we need to provide code sugars to simplify this scenario.

### **Scenario 4: serve CPU-intensive applications in multiple Napa containers.**
We can have multiple Napa containers running different apps within a single Node.JS instance. The benefit of using multiple Napa containers is that we can apply different runtime policies (like heapSize, etc.) to different containers.  

multi-container-app-server.js
```
var napa = require('napa');

napa.app.register('dispatcher');

// computationHeavyTask is a function defined in user-code-entry.js.
// args is an array of serializable objects that matches computationHeavyTask signature.
function handleRequest(jsonRequest) {
    napa.app.process(jsonRequest);
}
```

dispatcher.js
```
var napa = require('napa');
var path = require('path);
module.export = new napa.app.Application(path.resolve(__dirname, 'app.json'));
```

app.json
```
{
    "application": "dispatcher",
    "description": "dispatcher between production and experiment container."
    "namedObjects": ["entrypoints.json"]
}
```

entrypoints.json
```
[
    {
        "name": "serveRequest",
        "value": {
            "_type": "EntryPoint",
            "module": "dispatcher-entrypoints",
            "function": "serveRequest" 
        }
    }
]

```

dispatcher-entrypoints.js
```
var napa = require('napa');

// Create a Napa container with 8 threads, each thread with heap 128MB.
var prodContainer = napa.runtime.createContainer(
    {
        numberOfThreads: 8,
        heapSizeInMB: 128
    }
);

var expContainer = napa.runtime.createContainer(
    {
        numberOfThreads: 2,
        heapSizeInMB: 64,
        allowUnsafeCode: true
    }
);

// Load entry JavaScript file before execution.
// TODO: We need a better way to deal with app registration within containers.
prodContainer.load('app-set1.js');
expContainer.load('app-set2.js');

export function serveRequest(napa.app.RequestContext context) {
    var container = prodContainer;
    if (context.input.source == 'Experiment') {
        container = expContainer;
    }
    container.run('handleRequest', [jsonRequest], (req, res) => {
        if (status.code === napa.runtime.ResultCode.success) {
            return JSON.parse(response);
        }
        else {
            console.error(status.errorMessage);
            return null;
        }
    });
}

```

app-set1.js
```
var napa = require('napa');
napa.app.register('bing');
napa.app.register('malta');
handleRequest = napa.app.process;
```

app-set2.js
```
var napa = require('napa');
// We can run the same application in different container.
napa.app.register('bing');
napa.app.register('exp-malta');
handleRequest = napa.app.process;
```
<hr/>
END