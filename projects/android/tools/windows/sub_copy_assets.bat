@ECHO OFF

del /S /Q "..\..\assets\data\"
xcopy "..\..\..\..\executable\data" "..\..\assets\data\" /E