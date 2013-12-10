@ECHO OFF

CALL 0_paths.bat
CALL sub_change_folder.bat

CALL "%_PATH_NDK_%\ndk-build"

PAUSE