# namespace `store`

## Table of Contents
- [`create(id: string): Store`](#create)
- [`get(id: string): Store`](#get)
- [`getOrCreate(id: string): Store`](#getOrCreate)
- [`count: number`](#count)
- interface [`Store`](#store)
    - [`store.id: string`](#store-id)
    - [`store.set(key: string, value: any): void`](#store-set)
    - [`store.get(key: string): any`](#store-get)
    - [`store.has(key: string): boolean`](#store-has)
    - [`store.size: number`](#store-size)

## APIs
Store API is introduced as a necessary complement of sharing [transportable](transport.md#transportable) objects across JavaScript threads, on top of passing objects via arguments. During `store.set`, values marshalled into JSON and stored in process heap, so all threads can access it, and unmarshalled while users retrieve them via `store.get`.

Though very convenient, it's not recommended to use store to pass values within a transaction or request, since its overhead is more than passing objects by arguments (there are extra locking, etc.). Besides, developers have the obligation to delete the key after usage, while it's automatically managed by reference counting in passing arguments.

Following APIs are exposed to create, get and operate upon stores.

### <a name="create"></a>create(id: string): Store
It creates a store by a string identifer that can be used to get the store later. When all references to the store from all JavaScript VMs are cleared, the store will be destroyed. Thus always keep a reference at global or module scope is usually a good practice using `Store`. Error will be thrown if the id already exists.

Example:
```ts
let store = napa.store.create('store1');
```
### <a name="get"></a>get(id: string): Store
It gets a reference of store by a string identifier. `undefined` will be returned if the id doesn't exist. 

Example:
```ts
let store = napa.store.get('store1');
```

### <a name="getOrCreate"></a>getOrCreate(id: string): Store
It gets a reference of store by a string identifier, or creates it if the id doesn't exist. This API is handy when you want to create a store in code that is executed by every worker of a zone, since it doesn't break symmetry.

Example:
```ts
let store = napa.store.getOrCreate('store1');
```
### <a name="count"></a>count: number
It returns count of living stores.

### <a name="store"></a>interface `Store`
Interface that let user to put and get objects across multiple JavaScript VMs.

### <a name="store-id"></a>store.id: string
It gets the string identifier for the store.

### <a name="store-set"></a>store.set(key: string, value: any): void
It puts a [transportable](transport.md#transportable) value into store with a string key. If key already exists, new value will override existing value.

Example:
```ts
store.set('status', 1);
```
### <a name="store-get"></a>store.get(key: string): any
It gets a [transportable](transportable.md#transportable) value from the store by a string key. If key doesn't exist, `undefined` will be returned.

Example:
```ts
let value = store.get('status');
assert(value === 1);
```
### <a name="store-has"></a>store.has(key: string): boolean
It tells if a key exists in current store.

Example:
```ts
assert(store.has('status'))
```

### <a name="store-size"></a>store.size: number
It tells how many keys are stored in current store.