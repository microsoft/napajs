#!/usr/bin/env node

var log = require('npmlog');
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
		log.info('NAPA_INSTALL', 'trying to download the pre-build binaries.');
		execSync(fetchCommand, { 'stdio': 'inherit' });

		log.info('NAPA_INSTALL', 'done successfully by download.');
		skipBuild = true;
	} catch (e) {
		errorCode = e.status;
	}
}

// ==== Try to build from sources ====
if (!skipBuild) {
	try {
		log.info('NAPA_INSTALL', 'trying to build from sources.');
		execSync(buildCommand, { 'stdio': 'inherit' });

		log.info('NAPA_INSTALL', 'done successfully by build.');
	} catch (e) {
		errorCode = e.status;
	}
}

// ==== Compile Typescript files ====
if (errorCode == 0) {
	var npmVersion = execSync('npm --version').toString().trim();
	var npmMajorVersion = npmVersion.split('.')[0];
	var npmMajorVersionNumber = parseInt(npmMajorVersion);
	if (npmMajorVersionNumber < 4) {
		// Before npm 4.x, the 'prepare' script will not run automatically by npm.
		// We have to run it explicitly in this script.

		try {
			log.info('NAPA_INSTALL', `Current NPM version=${npmVersion}. NPM below 4.x does not recognize script 'prepare'.`);
			log.info('NAPA_INSTALL', 'running "npm run prepare"...');
			execSync('npm run prepare', { 'stdio': 'inherit' });
		} catch (e) {
			errorCode = e.status;
		}
	}
}

if (errorCode != 0) {
	log.error('NAPA_INSTALL', 'failed to install napajs.');
}

process.exit(errorCode);
