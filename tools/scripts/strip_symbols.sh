#!/bin/bash

_EXECUTABLE_="strip"
_PARAMETERS_="--strip-unneeded"

if [ -d "$1" ]; then

    find "$1" -iname "*.*" | while read file; do

        $_EXECUTABLE_ $_PARAMETERS_ "$file"

    done

else

    $_EXECUTABLE_ $_PARAMETERS_ "$1"

fi