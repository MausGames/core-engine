@ECHO OFF

CALL sub_set_paths.bat
CALL sub_change_folder.bat

ECHO sdk.dir=%_PATH_SDK_%> local.properties
CALL "%_PATH_ANT_%\ant" debug

PAUSE