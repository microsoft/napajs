#!/usr/bin/env node

var log = require('npmlog');
var fileExistsSync = require('fs').existsSync;
var path = require('path');
var execSync = require('child_process').execSync;

// Default command
var fetchCommand = 'node-pre-gyp install --fallback-to-build=false';
var buildCommand = 'cmake-js compile';

// ==== Determine install options ====
// Skip the fetch stage. '--no-fetch', or '--fetch=false'
var skipFetch = process.env.hasOwnProperty('npm_config_fetch') && !process.env['npm_config_fetch'];
// Skip the build stage. '--no-build', or '--build=false'
var skipBuild = process.env.hasOwnProperty('npm_config_build') && !process.env['npm_config_build'];

// Use debug build
if (process.env.hasOwnProperty('npm_config_debug') && process.env['npm_config_debug']) {
    buildCommand += " --debug";
}

log.info('NAPA_INSTALL', 'installing napajs...');

var errorCode = 0;

// ==== Try to fetch the pre-build binaries ====
if (!skipFetch) {
    try {
        log.info('NAPA_INSTALL', 'downloading the pre-build binaries...');
        execSync(fetchCommand, { 'stdio': 'inherit' });

        log.info('NAPA_INSTALL', 'completed successfully by download.');
        skipBuild = true;
    }
    catch (e) {
        errorCode = e.status;

        log.warn('NAPA_INSTALL', 'failed to download the pre-build binaries.');
        if (!skipBuild) {
            log.warn('NAPA_INSTALL', 'will fallback to build stage.');
        }
    }
}

// ==== Try to build from sources ====
if (!skipBuild) {
    errorCode = 0;

    try {
        log.info('NAPA_INSTALL', 'building from sources...');
        execSync(buildCommand, { 'stdio': 'inherit' });

        log.info('NAPA_INSTALL', 'completed successfully by build.');
    }
    catch (e) {
        errorCode = e.status;

        log.warning('NAPA_INSTALL', 'failed to build from sources.');
    }
}

// ==== Running "npm run prepare" explicitly ====
//
// NOTE: Napa.js has the "prepare" script, which is supposed to run by NPM
//       after a "npm install" command without parameters.
//       However, NPM below 4.x does not recognize script "prepare".
//       We have to run it explicitly in this case.
if (errorCode == 0) {
    var npmVersion = execSync('npm --version').toString().trim();
    log.info('NAPA_INSTALL', `current NPM version=${npmVersion}.`);

    var npmMajorVersion = npmVersion.split('.')[0];
    var npmMajorVersionNumber = parseInt(npmMajorVersion);
    if (npmMajorVersionNumber < 4) {
        log.info('NAPA_INSTALL', 'NPM below 4.x does not recognize script "prepare". We need to run it explicitly.');

        // Skip this step if we already have the TypeScripts compiled.
        var mainFilePath = path.join(__dirname, '..', process.env['npm_package_main']);
        if (fileExistsSync(mainFilePath) ) {
            log.info('NAPA_INSTALL', 'already have compiled typescripts. skip running "npm run prepare".');
        }
        else {
            log.info('NAPA_INSTALL', 'running "npm run prepare"...');

            try {
                execSync('npm run prepare', { 'stdio': 'inherit' });
            }
            catch (e) {
                errorCode = e.status;
            }
        }
    }
}

if (errorCode != 0) {
    log.error('NAPA_INSTALL', 'failed to install napajs.');
}

process.exit(errorCode);
