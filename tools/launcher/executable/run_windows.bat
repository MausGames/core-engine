@echo off

ver | findstr /i "5\.1\." > nul

IF %ERRORLEVEL% EQU 0 (

start /D "bin\windows\x86" CoreApp_mingw.exe

) ELSE (

start /D "bin\windows\x86" CoreApp_msvc.exe

)
