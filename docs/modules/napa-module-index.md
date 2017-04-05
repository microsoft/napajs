# Index of Modules@napajs

## Napa.JS foundation modules
- [napajs](../../modules/napajs/README.md) : A multi-thread JavaScript runtime on Node.
- [@napajs/config](../../modules/@napajs/config/README.md) : Utilities to read configuration files.
- [@napajs/lock](../../modules/@napajs/lock/README.md) : Synchronization between multiple isolates in Napa.JS
- [@napajs/logger](../../modules/@napajs/logger/README.md) : Access logging in Napa.JS. 
- [@napajs/memory](../../modules/@napajs/memory/README.md): Handling memories in Napa.JS.
- [@napajs/metrics](../../modules/@napajs/metrics/README.md) : Access metrics in Napa.JS.
- [@napajs/workflow](../../modules/@napajs/workflow/README.md) : (TBD) Flow control in Napa.JS

## Public
### General purpose framework
- [edge](): Interop with C# libraries
- [storage-client](): Common API to access key/value storage.
- [winery](./@napajs/vineyard/README.md): Application framework to build highly iterative applications.

### General machine learning
- [expression-compiler](): Compiler for generating execution sequence for a list of expressions.
- [tf-serve](): (TBD) Tensorflow serving library

## Microsoft internal
- [@ms/autopilot](): Common Autopilot APIs.
- [@ms/cosmos-client](): API to access cosmos.
- [@ms/object-store-client](): Access object stores.
- [@ms/sonoma](): Abstraction layer to serve Tensorflow and CNTK models.
- [@ms/storage-client](): General API to access KV stores.

## Bing internal
- [@bing/features](): Bing ranking features.
- [@bing/knowledge-models](): Bing knowledge models
- [@bing/query](): Bing query parser and query object.
- [@bing/query-rewrite](): (TBD) Bing query rewrite.
- [@bing/trie](): Read and write trie files.
- [@bing/tree-ensemble](): Boost gradient decision tree ensemble used in Bing.
- [@bing/word-breaker](): Bing word breaker.