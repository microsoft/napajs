@echo off

IF EXIST %NAPA_ROOT%\build\node\node.exe (
    SET NODE_EXE=%NAPA_ROOT%\build\node\node.exe
) ELSE (
    SET NODE_EXE=node.exe
)

%NODE_EXE% napa-cl.js %*