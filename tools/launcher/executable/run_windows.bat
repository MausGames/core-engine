@ECHO OFF

VER | FINDSTR /i "5\.1\." > NUL

IF %ERRORLEVEL% EQU 0 (

START /D "bin\windows\x86" CoreApp_mingw.exe

) ELSE (

START /D "bin\windows\x86" CoreApp_msvc.exe

)
