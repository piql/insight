setlocal
set IN_JPG=%1
set IN_ALTO=%2
set OUT=%3
set ERRORLOG=%4

python --help > nul
if not %ERRORLEVEL% equ 0 (
    echo %0: python not in path >> %ERRORLOG%
    EXIT /B 1
)

set HOCR_SCRIPT=.\HocrConverter\HocrConverter.py
if not exist %HOCR_SCRIPT% (
    echo ERROR %HOCR_SCRIPT% not found >> %ERRORLOG%
)

rem set DEBUG_PARAMS=-b -v -t

rem echo "alto2hocr.cmd %IN_ALTO% | python %HOCR_SCRIPT% -i - -I -V -n -o %OUT% %DEBUG_PARAMS% %IN_JPG%" >> %ERRORLOG%

alto2hocr.cmd %IN_ALTO% %errorlog% | python %HOCR_SCRIPT% -i - -I -V -n -o %OUT% %DEBUG_PARAMS% %IN_JPG% >> %ERRORLOG% 2>&1
if not %ERRORLEVEL% equ 0 (
    echo %0: ERROR executing "alto2hocr.cmd %IN_ALTO% | python %HOCR_SCRIPT% -i - -I -V -n -o %OUT% %DEBUG_PARAMS% %IN_JPG%" >> %ERRORLOG%
    exit /b 1
)
