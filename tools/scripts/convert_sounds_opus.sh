#!/bin/bash

_EXECUTABLE_="ffmpeg"
_PARAMETERS_="-c:a libopus -b:a 96k -vbr on"

if [ -d "$1" ]; then

    for file in "$1"/*.wav; do

        [ -e "$file" ] || continue
        $_EXECUTABLE_ -i "$file" $_PARAMETERS_ "${file%.*}.opus"

    done

else

    $_EXECUTABLE_ -i "$1" $_PARAMETERS_ "${1%.*}.opus"

fi