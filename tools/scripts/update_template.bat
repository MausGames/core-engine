@ECHO OFF

SET _EXECUTABLE_="xcopy"
SET _PARAMETERS_=/E /I /K /Y

IF "%~2" == "defaults" (
    REM not yet implemented
)
IF "%~2" == "libraries" (
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\template\executable\bin" "%~1\executable\bin"
)
IF "%~2" == "shaders" (
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\template\executable\data\shaders\global.glsl" "%~1\executable\data\shaders\global.glsl"
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\template\executable\data\shaders\readme.txt" "%~1\executable\data\shaders\readme.txt"
)