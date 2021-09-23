#!/bin/bash

_PATH_="$(dirname $(realpath $0))"

_EXECUTABLE_="rsync"
_PARAMETERS_="--recursive --update --mkpath --xattrs --times"

if [ "$2" == "defaults" ]; then
    : # not yet implemented
elif [ "$2" == "libraries" ]; then
    $_EXECUTABLE_ $_PARAMETERS_ "$_PATH_/../../libraries/bin/linux_x86_64/" "$1/executable/bin/linux_x86_64/"
    $_EXECUTABLE_ $_PARAMETERS_ "$_PATH_/../../libraries/bin/windows_x86_32/dll/" "$1/executable/bin/windows_x86_32/"
    $_EXECUTABLE_ $_PARAMETERS_ "$_PATH_/../../libraries/bin/windows_x86_64/dll/" "$1/executable/bin/windows_x86_64/"
elif [ "$2" == "shaders" ]; then
    $_EXECUTABLE_ $_PARAMETERS_ "$_PATH_/../template/executable/data/shaders/global.glsl" "$1/executable/data/shaders"
    $_EXECUTABLE_ $_PARAMETERS_ "$_PATH_/../template/executable/data/shaders/readme.txt" "$1/executable/data/shaders"
fi