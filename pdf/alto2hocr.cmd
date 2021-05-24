@echo off
setlocal
set XSLT_TOOL="C:\Program Files\Saxonica\SaxonHE10.5N\bin\transform"
set ALTO2HOCR_XSLT=alto__hocr.xsl

if "%2" NEQ "" SET OUTHOCR="-o:%2" 

%XSLT_TOOL% -s:%1 -xsl:%ALTO2HOCR_XSLT% %OUTHOCR%

  

