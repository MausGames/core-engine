@ECHO OFF

SET _EXECUTABLE_="xcopy"
SET _PARAMETERS_=/I /K /Y

IF "%~2" == "libraries" (
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\..\libraries\bin\linux_x86_64\" "%~1\executable\bin\linux_x86_64\"
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\..\libraries\bin\windows_x86_32\dll\" "%~1\executable\bin\windows_x86_32\"
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\..\libraries\bin\windows_x86_64\dll\" "%~1\executable\bin\windows_x86_64\"
)
IF "%~2" == "shaders" (
    CALL %_EXECUTABLE_% %_PARAMETERS_% "%~dp0..\template\executable\data\shaders\engine\" "%~1\executable\data\shaders\engine\"
)