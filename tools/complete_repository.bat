@ECHO OFF

CALL ..\tools\scripts\download_archive.bat core-engine libraries "..\libraries\bin"
CALL ..\tools\scripts\download_archive.bat core-engine tools "..\tools\scripts\base"