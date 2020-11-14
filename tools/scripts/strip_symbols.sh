#!/bin/bash

_EXECUTABLE_="strip"
_PARAMETERS_="--strip-unneeded"

if [ -d "$1" ]; then

    for file in $(find "$1" \( -name "*" \) -print0 | xargs -0); do

        $_EXECUTABLE_ $_PARAMETERS_ "$file"

    done

else

    $_EXECUTABLE_ $_PARAMETERS_ "$1"

fi