@ECHO OFF

SET VAR_STORE="My"
SET VAR_CERT=""

SET PATH_SIGN="C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe"
SET PATH_TIME="http://timestamp.digicert.com"

%PATH_SIGN% sign /s %VAR_STORE% /n %VAR_CERT% /fd SHA256 /tr %PATH_TIME% /td SHA256 "%~1"