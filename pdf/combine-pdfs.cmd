@echo off

setlocal EnableDelayedExpansion

set FOLDER=%1
set FILES=%2
set OUT=%3
set convertlog=%4

pushd %FOLDER%
pdftk %FILES% cat output %OUT% >> %convertlog%
if not %ERRORLEVEL% equ 0 (
    echo ERROR executing pdftk %FILES% cat output %OUT% >> %convertlog%
    exit /b 1
)
popd

endlocal
