#!/bin/bash

_PATH_="$(dirname "$(realpath "$0")")"

_EXECUTABLE_="cwebp"
_PARAMETERS_="-lossless -exact -q 100 -alpha_q 100 -m 6 -metadata none -progress -mt"

if [ -d "$1" ]; then

    find "$1" -iname "*.png" | while read file; do

        $_EXECUTABLE_ $_PARAMETERS_ "$file" -o "${file%.*}.webp"

    done

else

    $_EXECUTABLE_ $_PARAMETERS_ "$1" -o "${1%.*}.webp"

fi