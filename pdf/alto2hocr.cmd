@echo off
setlocal
set XSLT_TOOL="C:\Program Files\Saxonica\SaxonHE10.5N\bin\transform"
set ALTO2HOCR_XSLT=alto__hocr.xsl

set ERRORLOG=%2
shift /2

%XSLT_TOOL% -? > nul
if not %ERRORLEVEL% equ 0 (
    echo %0: %XSLT_TOOL% not in path >> %errorlog%
    EXIT /B 1
)

if not exist %ALTO2HOCR_XSLT% (
    echo %0: %ALTO2HOCR_XSLT% not in path >> %errorlog%
    EXIT /B 1
)


if "%2" NEQ "" SET OUTHOCR="-o:%2" 

echo %XSLT_TOOL% -s:%1 -xsl:%ALTO2HOCR_XSLT% %OUTHOCR% >> %errorlog%
%XSLT_TOOL% -s:%1 -xsl:%ALTO2HOCR_XSLT% %OUTHOCR%

  

