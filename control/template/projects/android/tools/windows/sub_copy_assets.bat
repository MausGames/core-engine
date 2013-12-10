@ECHO OFF

del /S /Q "..\..\project\assets\data\"
xcopy "..\..\..\..\executable\data" "..\..\project\assets\data\" /E