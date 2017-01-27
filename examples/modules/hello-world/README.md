# Hello World

This example shows the simple napa module, which shows the basic difference between node.js module and napa module.

```cpp
#include <napa-module.h>

namespace napa {
namespace demo {

using namespace v8;

void Method(const FunctionCallbackInfo<Value>& args) {
  auto isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, "world"));
}

void Init(Local<Object> exports) {
  NAPA_SET_METHOD(exports, "hello", Method);
}

NAPA_MODULE(addon, Init)

}  // namespace demo
}  // namespace napa
```

## Transition from node.js module
* *napa-module.h* is used instead of *node.h*. Depending on preprocessor definition, *NAPA_MODULE_EXTENSION* or
 *BUILDING_NODE_EXTENSION* preprocessor definition, *napa-module.h* includes necessary napa or node header files
  accordingly and build system creates either node.js module or napa module.
* *NAPA_SET_METHOD* is equivalent to *NODE_SET_METHOD*. This module will have *hello()* function.
* *NAPA_MODULE* is equivalent to *NODE_MODULE*, which exports an initialization function.

## Typescript
It's recommended that typescript or typescript definition is provided to let the user know the APIs without
 the source codes and develop Typescript project easily.
### hello-world.ts
```ts
var addon = require('../bin/addon');

export function hello(): string {
    return addon.hello();
}
```
### hello-world.d.ts
```d.ts
export declare function hello(): string;
```

## Mocha test
```js
var assert = require('assert');
var helloWorld = require('hello-world');

describe('Test suite for hello-word', function() {
    it('prints the string "world"', function() {
        var result = helloWorld.hello();
        assert.equal(result, 'world');
    });
})
```