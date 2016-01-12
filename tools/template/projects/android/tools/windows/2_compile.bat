@ECHO OFF

CALL sub_set_paths.bat
CALL sub_change_folder.bat

CALL "%_PATH_NDK_%\ndk-build" -j 4

PAUSE