@ECHO OFF

SET _EXECUTABLE_="%~dp0base\optipng.exe"
SET _PARAMETERS_=-strip all -nx -o5

IF EXIST "%~1\" (

    FOR /R "%~1\" %%G IN (*.png) DO CALL %_EXECUTABLE_% %_PARAMETERS_% "%%G"

) ELSE (

    CALL %_EXECUTABLE_% %_PARAMETERS_% "%1"

)