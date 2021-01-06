#!/bin/bash

_PATH_="$(dirname $(realpath $0))"

_EXECUTABLE_="cp"
_PARAMETERS_="--recursive --update --preserve"

if [ "$2" == "defaults" ]; then
    : # not yet implemented
elif [ "$2" == "libraries" ]; then
    $_EXECUTABLE_ $_PARAMETERS_ "$_PATH_/../template/executable/bin" "$1/executable"
elif [ "$2" == "shaders" ]; then
    $_EXECUTABLE_ $_PARAMETERS_ "$_PATH_/../template/executable/data/shaders/global.glsl" "$1/executable/data/shaders"
    $_EXECUTABLE_ $_PARAMETERS_ "$_PATH_/../template/executable/data/shaders/readme.txt" "$1/executable/data/shaders"
fi