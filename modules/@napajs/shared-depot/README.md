# @napajs/shared-depot

It introduces three components to make native objects shared among isolates.

* *SharedWrap*: It's a Javascript object wrapping a shared pointer to native object. Each instance is stored and accessed by a unique name.
* *SharedBarrel*: A shared barrel holds multiple SharedWrap instances. Each shared barrel has a unique id assigned by shared depot.
* *SharedDepot*: It has a set of shared barrels. Only one shared depot instance exists.

```diagram
SharedDepot
------------------------------------------------------------------------------------------------------------------------
|                                                                                                                      |
|         SharedBarrel_1             SharedBarrel_2             SharedBarrel_3                   SharedBarrel_n        |
|   |------------------------| |------------------------| |------------------------|       |------------------------|  |
|   | name_11, SharedWrap_11 | | name_21, SharedWrap_21 | | name_31, SharedWrap_31 |       | name_n1, SharedWrap_n1 |  |
|   |------------------------| |------------------------| |------------------------|       |------------------------|  |
|   | name_12, SharedWrap_12 | | name_22, SharedWrap_22 | | name_32, SharedWrap_32 |       | name_n2, SharedWrap_n2 |  |
|   |------------------------| |------------------------| |------------------------|       |------------------------|  |
|   | name_13, SharedWrap_13 | | name_23, SharedWrap_23 | | name_33, SharedWrap_33 |       | name_n3, SharedWrap_n3 |  |
|   |------------------------| |------------------------| |------------------------|       |------------------------|  |
|   |            .           | |            .           | |            .           | ..... |            .           |  | 
|   |            .           | |            .           | |            .           |       |            .           |  | 
|   |            .           | |            .           | |            .           |       |            .           |  | 
|   |------------------------| |------------------------| |------------------------|       |------------------------|  |
|   | name_1m, SharedWrap_1m | | name_2m, SharedWrap_2m | | name_3m, SharedWrap_3m |       | name_nm, SharedWrap_nm |  |
|   |------------------------| |------------------------| |------------------------|       |------------------------|  |
|                                                                                                                      |
------------------------------------------------------------------------------------------------------------------------
```

## SharedWrap

This is an example to show how to create *SharedWrap* instance.

```cpp
#include <napa-log.h>
#include <napa/shared-wrap.h>

using namespace napa;
using namespace napa::module;

NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR

struct SharedString {

    void Set(const char* value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _value.assign(value);
    }

    std::string Get() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _value;
    }

    std::mutex _mutex;
    std::string _value;
};

// Create a dummy SharedWrap object for test.
void CreateTestableSharedString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    auto objectWrap = SharedWrap::NewInstance(std::make_shared<SharedString>());
    args.GetReturnValue().Set(objectWrap);
}

void SetTestableSharedString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 2,
        "Two arguments are required.");

    CHECK_ARG(isolate,
        args[0]->IsObject(),
        "Shared object must be given as the first argument.");

    CHECK_ARG(isolate,
        args[1]->IsString(),
        "String must be given as the second argument.");

    auto objectWrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args[0]->ToObject());
    assert(objectWrap != nullptr);
    auto object = objectWrap->Get<SharedString>();

    v8::String::Utf8Value value(args[1]->ToString());
    object->Set(*value);

    args.GetReturnValue().SetUndefined();
}

void ReadTestableSharedString(const v8::FunctionCallbackInfo<v8::Value>& args) {
    auto isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);

    CHECK_ARG(isolate,
        args.Length() == 1 && args[0]->IsObject(),
        "Shared object must be given as an argument.");

    auto objectWrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args[0]->ToObject());
    NAPA_ASSERT(objectWrap != nullptr, "Can't unwrap a shared object of SharedString type.");
    auto object = objectWrap->Get<SharedString>();
    std::string value = object->Get();

    args.GetReturnValue().Set(v8_helpers::MakeV8String(isolate, value.c_str()));
}

void InitAll(v8::Local<v8::Object> exports) {
    NAPA_SHARED_WRAP_INIT();

    NAPA_SET_METHOD(exports, "createTestableSharedString", CreateTestableSharedString);
    NAPA_SET_METHOD(exports, "setTestableSharedString", SetTestableSharedString);
    NAPA_SET_METHOD(exports, "readTestableSharedString", ReadTestableSharedString);
}

NAPA_MODULE(dummy, InitAll);
```

Two macros must be specified to create a valid *SharedWrap* constructor.

```cpp
NAPA_DEFINE_PERSISTENT_SHARED_WRAP_CONSTRUCTOR
```
It defines a static instance of persistent constructor.

```cpp
NAPA_SHARED_WRAP_INIT();
```
It creates a persistent constructor at the current V8 context.

This API helps *SharedWrap* create or hold a native object.
```cpp
template <typename T>
static v8::Local<v8::Object> Create(std::shared_ptr<T> object);
```

Similar to node.js's *ObjectWrap* class, *Unwrap* function returns *SharedWrap* instance.
```cpp
auto sharedWrap = NAPA_OBJECTWRAP::Unwrap<SharedWrap>(args[0]->ToObject());
```

*SharedWrap::Get()* returns a shared pointer to native object.
```cpp
template <typename T = void>
std::shared_ptr<T> Get();
```

## SharedBarrel

It provides APIs to access *SharedWrap* instances.

```js
id()
```
It gives a shared barrel id assigned by *SharedDepot.assign()*.


```js
exists(key)
```
It returns true if *ShardWrap* instnace with a given key exists.

```js
set(key, object)
```
It stores a *SharedWrap* instance into a shared barrel. *key* is a unique name for a shareb object, *object*.
 It returns true when an object is stored successfully. If the same key exists or it fails, return false.

```js
get(key)
```
It returns a *SharedWrap* instance.

```js
remove(key)
```
It removes a given *SharedWrap* instance from a shared barrel.

```js
count()
```
It returns the number of *SharedWrap* instances in a shared barrel.


## SharedDepot

It provides APIs to access *SharedBarrel* instances.

```js
assign()
```
It returns a *SharedBarrel* instance and assigns a unique numeric Id, which can get by *SharedBarrel.id()*

```js
release(store)
```
It release a *SharedBarrel* instance.

```js
find(id)
```
It finds a *SharedBarrel* instance using a given *id*. It returns null at no finding.

```js
count()
```
It gives the number of *SharedBarrel* instances.

## Example

This is the mocha test to show how to use *@napajs/shared-depot* module.

```js
var assert = require('assert');
var sharedDepot = require('@napajs/shared-depot');
var dummy = require('./dummy');

describe('Test suite for @napajs/shared-depot', function() {
    var dummyKey: string = 'dummy';

    it('creates a barrel', function() {
        var barrel = sharedDepot.assign();
        assert(barrel != null);
        assert.equal(sharedDepot.count(), 1);
    })

    it('creates a shared string and adds into a shared barrel', function() {
        var barrel = sharedDepot.assign();

        var sharedString: any = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'napa');

        assert(!barrel.exists(dummyKey));

        var succeeded: boolean = barrel.set(dummyKey, sharedString);
        assert(succeeded);
        assert.equal(barrel.count(), 1);

        assert(barrel.exists(dummyKey));

        sharedString = barrel.get(dummyKey);
        assert(sharedString != null);

        var result: string = dummy.readTestableSharedString(sharedString);
        assert.equal(result, 'napa');
    });

    it('restores a barrel using identifier', function() {
        var barrel = sharedDepot.assign();

        var sharedString: any = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'napa');
        var succeeded: boolean = barrel.set(dummyKey, sharedString);
        assert(succeeded);

        var restored: any = sharedDepot.find(barrel.id());
        assert(restored != null);

        sharedString = restored.get(dummyKey);
        assert(sharedString != null);

        var result: string = dummy.readTestableSharedString(sharedString);
        assert.equal(result, 'napa');
    });

    it('assigns and releases a barrel', function() {
        var barrel = sharedDepot.assign();

        var anotherBarrel: any = sharedDepot.assign();
        assert.equal(sharedDepot.count(), 2);

        sharedDepot.release(anotherBarrel);
        assert.equal(sharedDepot.count(), 1);
    });

    it('adds two shared strings into a barrel', function() {
        var barrel = sharedDepot.assign();
        var sharedString: any = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'napa');

        var succeeded: boolean = barrel.set(dummyKey, sharedString);
        assert(succeeded);
        assert.equal(barrel.count(), 1);

        assert(barrel.exists(dummyKey));

        sharedString = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'vteam');

        succeeded = barrel.set(dummyKey, sharedString);
        assert(!succeeded);
        assert.equal(barrel.count(), 1);

        succeeded = barrel.set(dummyKey + '1', sharedString);
        assert(succeeded);
        assert.equal(barrel.count(), 2);
        
        assert(barrel.exists(dummyKey + '1'));
    });

    it('deletes a shared string from a barrel', function() {
        var barrel = sharedDepot.assign();
        var sharedString: any = dummy.createTestableSharedString();
        dummy.setTestableSharedString(sharedString, 'napa');

        barrel.remove(dummyKey);

        var sharedString: any = barrel.get(dummyKey);
        assert(sharedString == null);

        assert.equal(barrel.count(), 0);
    });
})
```