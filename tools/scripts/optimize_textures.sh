#!/bin/bash

_EXECUTABLE_="./base/optipng.elf"
_PARAMETERS_="-strip all -nx -o5"

if [ -d "$1" ]; then

    for file in $(find "$1" \( -name "*.png" \) -print0 | xargs -0); do

        $_EXECUTABLE_ $_PARAMETERS_ "$file"

    done

else

    $_EXECUTABLE_ $_PARAMETERS_ "$1"

fi