:: Copyright (c) Microsoft Corporation. All rights reserved.
:: Licensed under the MIT license.
@echo off

set NODE_EXE=%~dp0\node.exe
if not exist "%NODE_EXE%" (
  set NODE_EXE=node
)

%NODE_EXE% %~dp0\build.js %*
