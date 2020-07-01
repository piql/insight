
@echo off

:: This ensures the parameters are resolved prior to the internal variable
::
SetLocal EnableDelayedExpansion

:: Replaces Key Variables
::
:: Parameters:
:: %1  = Line to search for replacement
:: %2  = Key to replace
:: %3  = Value to replace key with
:: %4  = File in which to write the replacement
::

:: Read in line without the surrounding double quotes (use ~)
::
set line=%~1

:: Write line to specified file, replacing key (%2) with value (%3)
::
echo !line:%2=%3! >> %4

:: Restore delayed expansion
::
EndLocal
