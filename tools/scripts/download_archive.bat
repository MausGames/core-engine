@ECHO OFF

SET _PROJECT_=%~1
SET _FILE_=%~2.zip
SET _TARGET_=%~3

MKDIR "%_TARGET_%"

curl --user-agent "MausGames" --location "https://maus-games.at/data/%_PROJECT_%/%_FILE_%" --output "%_TARGET_%\%_FILE_%"
tar -vvv --keep-newer-files --extract --directory="%_TARGET_%" --file="%_TARGET_%\%_FILE_%"

DEL "%_TARGET_%\%_FILE_%"