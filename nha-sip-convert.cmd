setlocal
set IN=%1
echo %0
echo %dirname%

for %%F in (%0) do set dirname=%%~dpF

; set PATH=%PATH%;%dirname%
cd %dirname%


set timestamp=%date%_%time:~0,-3%
set timestamp=%timestamp::=_%
set timestamp=%timestamp:.=_%
set timestamp=%timestamp:/=_%
set timestamp=%timestamp: =_%

set OUT=export-%timestamp%.pdf

set PLATFORM=-platform windows:dpiawareness=0

insight %PLATFORM% --file %IN% --file-format "Norsk Helsearkiv SIP" --auto-export %OUT%

pause
