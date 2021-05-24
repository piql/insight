@echo off
setlocal
set IN_JPG=%1
set IN_ALTO=%2
set OUT=%3

rem set DEBUG_PARAMS=-b -v -t

alto2hocr.cmd %IN_ALTO% | python .\HocrConverter\HocrConverter.py -i - -I -V -n -o %OUT% %DEBUG_PARAMS% %IN_JPG% 
