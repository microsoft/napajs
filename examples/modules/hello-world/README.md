# Hello World

This example shows the simple napa module, which shows the basic difference between node.js module and napa module.

```cpp
#include <napa/module.h>

namespace napa {
namespace demo {

using namespace v8;

void Method(const FunctionCallbackInfo<Value>& args) {
    auto isolate = args.GetIsolate();
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "world"));
}

void Init(Local<Object> exports) {
    NODE_SET_METHOD(exports, "hello", Method);
}

NODE_MODULE(addon, Init)

}  // namespace demo
}  // namespace napa
```

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