@echo off

ver | findstr /i "5\.1\." > nul

IF %ERRORLEVEL% EQU 0 (

start /D "bin\windows" CoreApp_mingw.exe

) ELSE (

start /D "bin\windows" CoreApp_msvc.exe

)
