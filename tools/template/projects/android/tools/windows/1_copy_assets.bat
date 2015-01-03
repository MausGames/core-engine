@ECHO OFF

del /S /Q "..\..\project\assets\data\"
xcopy "..\..\..\..\executable\data" "..\..\project\assets\data\" /E
xcopy "..\..\override" "..\..\project\assets\data\" /E /Y