
@if "%1" EQU "" (
    @echo Usage: %0 ^<version^> 
    @goto :eof
)

:: Translate
lupdate insight.pro
set /p CHANGE=If lupdate reports any changes, abort release and run linguist tool.

:: Build
nmake release

set appname=insight
set app=%appname%.exe
set targetname=insight
set targetapp=%targetname%.exe
set target=innsyn-%1

rd /q/s %target%
mkdir %target%

:: APP
copy release\%app% %target%\%targetapp% 
copy release\%appname%.pdb %target%\%targetname%.pdb 
copy readme.txt %target%\.
copy lesmeg.txt %target%\.
copy %appname%.conf %target%\%targetname%.conf
copy sphinx\* %target%\.
copy src\thirdparty\pdf2text\release\pdf2text.exe %target%\.
copy src\thirdparty\create_xml\release\create_xml.exe %target%\.
copy release\POPPLER-QT5.DLL %target%\.
copy release\QT5GUI.DLL %target%\.
copy release\QT5CORE.DLL %target%\.
copy release\QT5XML.DLL %target%\.
copy c:\windows\system32\MSVCP120.DLL %target%\.
copy c:\windows\system32\MSVCR120.DLL %target%\.
copy *qm %target%\.

:: FORMATS
xcopy /s/i formats %target%\formats

:: TESTDATA
xcopy /s/i testdata %target%\testdata

set tgz=%target%.tgz
tar czf %tgz% %target%/*
