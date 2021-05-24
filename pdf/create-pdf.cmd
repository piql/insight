@echo off

rem CD to batch file location
pushd %~dp0

rem Create PDFs with OCR data.
rem   - Create PDF of each input JPG file
rem   - Add OCR to each PDF
rem   - Combine PDFs to output PDF
rem
rem  Tools:
rem   - ALTO to hOCR XSLT: https://github.com/filak/hOCR-to-ALTO
rem   - hOCR + JPG to PDF: HocrConverter python script https://github.com/piql/HocrConverter.git
rem   - Combine PDFs: https://www.pdflabs.com/tools/pdftk-server/

setlocal EnableDelayedExpansion

set out=%1
set files=%2
set temp=%3

set timestamp=%date%_%time:~0,-3%
set timestamp=%timestamp::=_%
set timestamp=%timestamp:.=_%
set convertlog=%temp%\journal_%timestamp%.log

echo Exporting journal to PDF > %convertlog%
echo Parameters: >> %convertlog%
echo    out PDF: %out% >> %convertlog%
echo    pages: %files% >> %convertlog%
echo    dir: %temp% >> %convertlog%
echo. >> %convertlog%

set output=%temp%\journal_%timestamp%
set pdffiles=%temp%\journal_pages_%timestamp%.txt
set n=0
FOR /F "tokens=1,2 delims=;" %%i in (%files%) do (
   set outpdf=%output%_!n!.pdf
   echo Converting page: %%i ocr: %%j to !outpdf! >> %convertlog%
   call alto2pdf.cmd %%i %%j !outpdf!

   echo !outpdf! >> %pdffiles%
   call set pdfs=%%pdfs%% !outpdf!
   set /A n+=1   
)

pdftk %pdfs% output %out%

endlocal
popd