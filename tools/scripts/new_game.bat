@ECHO OFF


SET /p PRJ_NAME=Name of the new game project: 

SET PRJ_CUR=%CD%\..\template
CD ../../..
SET PRJ_TARGET=%CD%\%PRJ_NAME%\

SET /p PRJ_CONFIRM=Create folder ^<%PRJ_TARGET%^> [y/n]: 
IF NOT %PRJ_CONFIRM:~0,1% == y EXIT


ECHO Copying...


xcopy "%PRJ_CUR%" "%PRJ_TARGET%" /E /Q


ECHO Renaming...


SET TOOL_REPLACE="%PRJ_CUR%\..\scripts\base\replace_text.bat"
SET TOOL_NAME1=CoreApp
SET TOOL_NAME2=Core App
SET TOOL_TEMP=temp_file

SET PATH_CMAKE=%PRJ_TARGET%\projects\cmake
SET PATH_VISUALSTUDIO=%PRJ_TARGET%\projects\visualstudio

CD "%PATH_CMAKE%"
CALL RENAME CMakeLists.txt %TOOL_TEMP%
CALL %TOOL_REPLACE% %TOOL_NAME1% %PRJ_NAME% %TOOL_TEMP% > CMakeLists.txt
CALL DEL %TOOL_TEMP%

CD "%PATH_VISUALSTUDIO%\configuration"
CALL RENAME manifest.xml %TOOL_TEMP%
CALL %TOOL_REPLACE% %TOOL_NAME2% %PRJ_NAME% %TOOL_TEMP% > manifest.xml
CALL DEL %TOOL_TEMP%
CALL RENAME project.vcxproj %TOOL_TEMP%
CALL %TOOL_REPLACE% %TOOL_NAME1% %PRJ_NAME% %TOOL_TEMP% > project.vcxproj
CALL DEL %TOOL_TEMP%
CALL RENAME version.rc %TOOL_TEMP%
CALL %TOOL_REPLACE% %TOOL_NAME2% %PRJ_NAME% %TOOL_TEMP% > version.rc
CALL DEL %TOOL_TEMP%

CD "%PATH_VISUALSTUDIO%"
CALL %TOOL_REPLACE% %TOOL_NAME1% %PRJ_NAME% %TOOL_NAME1%.sln > %PRJ_NAME%.sln
CALL DEL %TOOL_NAME1%.sln

CD "%PRJ_TARGET%"
CALL RENAME .gitignore %TOOL_TEMP%
CALL %TOOL_REPLACE% %TOOL_NAME1% %PRJ_NAME% %TOOL_TEMP% > .gitignore
CALL DEL %TOOL_TEMP%

CD "executable"
CALL RENAME %TOOL_NAME1%_linux.sh %PRJ_NAME%_linux.sh
CALL RENAME %TOOL_NAME1%_windows.exe %PRJ_NAME%_windows.exe


ECHO ^<%PRJ_TARGET%^> created
%SystemRoot%\explorer.exe "%PRJ_TARGET%"


PAUSE