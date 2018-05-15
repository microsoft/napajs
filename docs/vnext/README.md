# Napa.js vNext (0.4.x-pre)
vNext is a version with new worker implementation. In Napa.js vNext, napa worker loads and initializes its javascript VM in node.js style, which enables most of node.js APIs and addon support.

## What's new in vNext
- Better Node.js API compatibility. Most of [Node APIs](https://nodejs.org/api/index.html) will be supported by default. See [compatibility tracking](./node-api-compatibility.md).
- Native addon support. All native addon can be loaded in worker by default, unless it's implemented non-thread-safely. See [compatibility tracking](./node-native-addon-compatibility.md).

See also: [Open Issues](https://github.com/Microsoft/napajs/issues?utf8=%E2%9C%93&q=is%3Aissue+is%3Aopen+project%3AMicrosoft%2Fnapajs%2F4), [Project vNext](https://github.com/Microsoft/napajs/projects/4), [Napa.js versions](./napa-versions.md)

## Install
Napa.js vNext requires **Node.js v10.2.0 or later**.

Install the latest pre release version:
```
npm install napajs@pre
```

Or [build from source](https://github.com/Microsoft/napajs/wiki/build-napa.js) (use branch `v0.4.x-pre` instead of `master`).
