@ECHO OFF

CALL sub_change_folder.bat

RMDIR /S /Q "assets/"
RMDIR /S /Q "gen/"
RMDIR /S /Q "obj/"

PAUSE