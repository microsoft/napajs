// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

'use strict';

const express = require('express');
const http = require('http');
const morgan = require('morgan');
const createError = require('http-errors');
let debug=require('debug')('napa:sample:reverseproxy:server');

const app = express();
let logger = morgan('combined');

debug(`server got argv as: `, process.argv);
const myargs = process.argv.slice(2);
const port = myargs.shift();
const docRoot = myargs.shift();

app.use(logger);

app.get('/info', function(req, res) {
    res.send(`Server@${port} serving:${docRoot}`);
});

app.use('/', express.static(docRoot));

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

  
debug(`Starting Server@${port} serving:${docRoot}...`);
app.set('port', port);
const server = http.createServer(app);
server.listen(port);
