@echo off

IF EXIST %NAPA_ROOT%\build\node\node.exe (
    SET NODE_EXE=%NAPA_ROOT%\build\node\node.exe
) ELSE (
    SET NODE_EXE=node.exe
)

IF EXIST %NAPA_ROOT%\build\typescript\lib\tsc.js (
    SET TSC=%NODE_EXE% %NAPA_ROOT%\build\typescript\lib\tsc.js
) ELSE (
    SET TSC=tsc
)

IF NOT EXIST napa-command-line.js (
    ECHO Compiling napa-command-line.ts...
    %TSC% napa-command-line.ts
)

%NODE_EXE% napa-command-line.js %*