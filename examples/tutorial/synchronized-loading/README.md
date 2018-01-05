# Synchronized Loading
This example implements a shared phone book component. The component will not load data until the first lookup request happens. When it starts to load data, it ensures the loading will be run only once. 

The component is implemented using lazy-loading pattern with the use of [`napa.sync.Lock`](./../../../docs/api/sync.md#interface-lock).

## How to run
1. Go to directory of "examples/tutorial/synchronized-loading"
2. Run "npm install" to install napajs
3. Run "node synchronized-loading.js"

## Program output
The output below shows one possible result. The sequence of the output may be different but the data loading will always run once only.
```
[lookupPhoneNumber] Start to lookup phone number of david.
[lookupPhoneNumber] Start to lookup phone number of wade.
[load_data] loading...
[lookupPhoneNumber] Start to lookup phone number of lisa.
[lookupPhoneNumber] wade : <not found>.
[lookupPhoneNumber] lisa : 567-888-9999.
[lookupPhoneNumber] david : 123-444-5555.
[run] All operations are completed.
```
