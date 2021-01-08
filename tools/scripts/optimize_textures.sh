#!/bin/bash

_EXECUTABLE_="./base/optipng.elf"
_PARAMETERS_="-strip all -nx -o5"

if [ -d "$1" ]; then

    find "$1" -iname "*.png" | while read file; do

        $_EXECUTABLE_ $_PARAMETERS_ "$file"

    done

else

    $_EXECUTABLE_ $_PARAMETERS_ "$1"

fi