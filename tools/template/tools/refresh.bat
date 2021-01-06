@ECHO OFF

CALL ..\..\CoreEngine\tools\scripts\update_template.bat "%CD%\.." defaults
CALL ..\..\CoreEngine\tools\scripts\update_template.bat "%CD%\.." libraries
CALL ..\..\CoreEngine\tools\scripts\update_template.bat "%CD%\.." shaders