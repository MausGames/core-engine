#!/bin/bash

_PROJECT_="$1"
_FILE_="$2.zip"
_TARGET_="$3"

mkdir "$_TARGET_"

curl --user-agent "MausGames" --location "https://maus-games.at/data/$_PROJECT_/$_FILE_" --output "$_TARGET_/$_FILE_"
unzip "$_TARGET_/$_FILE_" -d "$_TARGET_"

rm "$_TARGET_/$_FILE_"