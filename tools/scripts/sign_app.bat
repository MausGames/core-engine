@ECHO OFF

SET VAR_STORE=""
SET VAR_CERT=""

SET TOOL_SIGN="C:\Program Files (x86)\Windows Kits\8.1\bin\x64\signtool.exe"
SET TOOL_TIME="http://timestamp.verisign.com/scripts/timstamp.dll"

%TOOL_SIGN% sign /s %VAR_STORE% /n %VAR_CERT% "%1"
%TOOL_SIGN% timestamp /t %TOOL_TIME% "%1"
