@ECHO OFF

SET VAR_STORE="My"
SET VAR_CERT=""

SET PATH_SIGN="C:\Program Files (x86)\Windows Kits\8.1\bin\x64\signtool.exe"
SET PATH_TIME="http://timestamp.verisign.com/scripts/timstamp.dll"

%PATH_SIGN% sign /s %VAR_STORE% /n %VAR_CERT% /t %PATH_TIME% "%1"