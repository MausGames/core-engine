#!/bin/bash

_EXECUTABLE_="patchelf"
_PARAMETERS_="--set-soname"

if [ -d "$1" ]; then

    find "$1" -iname "*.so*" | while read file; do

        $_EXECUTABLE_ $_PARAMETERS_ "$(basename "$file")" "$file"

    done

else

    $_EXECUTABLE_ $_PARAMETERS_ "$(basename "$1")" "$1"

fi