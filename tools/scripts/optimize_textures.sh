#!/bin/bash

_PATH_="$(dirname $(realpath $0))"

_EXECUTABLE_="$_PATH_/base/optipng.elf"
_PARAMETERS_="-strip all -fix -nx -o5 -zm1-9 -i0"

if [ -d "$1" ]; then

    find "$1" -iname "*.png" | while read file; do

        $_EXECUTABLE_ $_PARAMETERS_ "$file"

    done

else

    $_EXECUTABLE_ $_PARAMETERS_ "$1"

fi