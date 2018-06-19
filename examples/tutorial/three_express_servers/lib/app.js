// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

"use strict";

const express = require('express');
const http = require('http');
const createError = require('http-errors');
const path = require('path');
const httpProxy = require('http-proxy');
const morgan = require('morgan');
const debug = require('debug')('napa:sample:reverseproxy:proxy');
const napa = require('../../../../lib');
const logger = morgan('tiny');

process.on('unhandledRejection', (reason, p) => {
  console.error(reason, 'Unhandled Rejection at Promise', p);
  process.exit(1);
});

process.on('uncaughtException', err => {
  console.error(err, 'Uncaught Exception thrown');
  process.exit(1);
});

let totalActiveZone = 0;
let finalExitCode = 0;

function startServer(port, docRoot) {
    totalActiveZone++;
    const zoneName = `serverZone${port}`;
    const serverZone = napa.zone.create(zoneName, {workers : 1});

    // setting argv(s) in napa worker(s)
    const set_args_command = 
        `process.argv = ["node", "server.js", "${port}", "${docRoot}"];`;
    serverZone.broadcast(set_args_command);

    // //Setting process.reallyExit() in worker(s)
    // const command_process_exit=
    //     'process.reallyExit=function(code) {' +
    //     '  let nodezone = napa.zone.node;' +
    //     '  nodezone.broadcast(`console.log("----Server exit(${code})");`);' +
    //     '};';
    // serverZone.broadcast(command_process_exit);

    const serverScriptPath = path.resolve('lib/server.js');
    serverZone.broadcast(`require('${serverScriptPath}');`);
    
    // serverZone.on("terminated", (exit_code) => {
    //     console.log(`Got zone:${zoneName} terminated with ${exit_code}`);
    //     if (finalExitCode == 0 && exit_code != 0) {
    //         finalExitCode = exit_code;
    //     }
    //     totalActiveZone--;
    //     if (totalActiveZone == 0) {
    //         process.exit(finalExitCode);
    //     }
    // });

    // debug('Let server zone feel free to exit()...')
    // serverZone.recycle();
}

startServer(3001, path.resolve('docs/server1'));
startServer(3002, path.resolve('docs/server2'));

const proxy = httpProxy.createProxyServer({
    ignorePath: true
});

proxy.on('error', function(err, req, res) {
    res.writeHead(500, {
        'Content-Type' : 'text/plain'
    });
    res.end(`http proxy error on ${req.url}`);
});

const app = express();

app.use(logger);

app.all('/app1/*', function(req, res) {
    const targetPath = req.path.substr(5);
    proxy.web(req, res, {target: `http://localhost:3001/${targetPath}`});
});

app.all('/app2/*', function(req, res) {
    const targetPath = req.path.substr(5);
    proxy.web(req, res, {target: `http://localhost:3002/${targetPath}`});
});

const myindex =
    '<html><body><ul>' +
    '  <li><a href="/app1/info">app1 info</a></li>' +
    '  <li><a href="/app1/">app1 root</a></li>' +
    '  <li><a href="/app2/info">app2 info</a></li>' +
    '  <li><a href="/app2/">app2 root</a></li>' +
    '</ul></body></html>';
app.get('/?', function(req, res) {
    res.send(myindex);
});

// catch 404 and forward to error handler
app.use(function (req, res, next) {
    next(createError(404));
});
  
// error handler
app.use(function (err, req, res, next) {
    // set locals, only providing error in development
    res.locals.message = err.message;
    res.locals.error = req.app.get('env') === 'development' ? err : {};

    // render the error page
    res.status(err.status || 500);
    res.render('error');
});


const server = http.createServer(app);
server.listen(3000);

console.log("Please visit http://localhost:3000/ ");
