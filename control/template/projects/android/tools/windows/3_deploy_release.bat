@ECHO OFF

CALL 0_paths.bat
CALL sub_change_folder.bat

CALL "%_PATH_ANT_%\ant" release install

PAUSE