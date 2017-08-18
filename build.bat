:: Copyright (c) Microsoft Corporation. All rights reserved.
:: Licensed under the MIT license.
@echo off

set NODE_EXE=%~dp0\node.exe
if not exist "%NODE_EXE%" (
  SET NODE_EXE=node
)

%NODE_EXE% %~dp0\scripts\build.js %*
