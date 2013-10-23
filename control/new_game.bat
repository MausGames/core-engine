@ECHO OFF

SET /p _PRJ_NAME_=Name of the game project: 

SET _PRJ_CUR_=%CD%\template
cd ../..
SET _PRJ_TARGET_=%CD%\%_PRJ_NAME_%\

SET /p _PRJ_CONFIRM_=Create folder ^<%_PRJ_TARGET_%^> [y/n]: 

IF %_PRJ_CONFIRM_:~0,1%==y xcopy "%_PRJ_CUR_%" "%_PRJ_TARGET_%" /E