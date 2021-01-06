@ECHO OFF

SET _EXECUTABLE_="xcopy"
SET _PARAMETERS_=/E /I /K /Y

IF "%~2" == "defaults" (
    REM not yet implemented
)
IF "%~2" == "libraries" (
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\..\libraries\bin\linux\x64\" "%~1\executable\bin\linux\x64\"
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\..\libraries\bin\windows\x64\dll\" "%~1\executable\bin\windows\x64\"
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\..\libraries\bin\windows\x86\dll\" "%~1\executable\bin\windows\x86\"
)
IF "%~2" == "shaders" (
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\template\executable\data\shaders\global.glsl" "%~1\executable\data\shaders\global.glsl"
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\template\executable\data\shaders\readme.txt" "%~1\executable\data\shaders\readme.txt"
)