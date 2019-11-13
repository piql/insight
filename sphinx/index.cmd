@echo off

set REPORTS=%1
set ATTACHMENTS=%2
set NAME=%3
set CONFIG=%REPORTS%sphinx.conf
set INDEX_CONFIG=%REPORTS%sphinx_index.conf
set INDEX=%REPORTS%sphinx\test1
set SOURCE_NAME=src_%NAME%
set INDEX_NAME=%NAME%

mkdir %INDEX%

:: Create sphinx config file in reports folder
DEL %INDEX_CONFIG%
setLocal EnableDelayedExpansion
For /f "tokens=* delims= " %%a in (sphinx_index.conf) do (
Set str=%%a
set str=!str:SOURCE_DIR=%ATTACHMENTS%!
set str=!str:SOURCE_NAME=%SOURCE_NAME%!
set str=!str:INDEX_DIR=%INDEX%!
set str=!str:INDEX_NAME=%INDEX_NAME%!
echo.!str!>>%INDEX_CONFIG%
)
ENDLOCAL

more %INDEX_CONFIG% > %CONFIG%
more sphinx.conf >> %CONFIG%

::sed -i 's/SOURCE_DIR/%ATTACHMENTS%/g' %CONFIG%
::sed -i 's/INDEX_DIR/%REPORTS%\\index/g' %CONFIG%

:: Run indexer
indexer.exe --config %CONFIG% --all > %REPORTS%\indexer.log
