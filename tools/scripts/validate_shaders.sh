#!/bin/bash

_PATH_="$(dirname "$(realpath "$0")")"

_EXECUTABLE_="$_PATH_/base/glslang.elf"
_PARAMETERS_="-t"

_GLOBAL_="$1/global.glsl"
_CUSTOM_="$1/custom.glsl"
_LOG_="$1/../shader_log.txt"

_VERSION_LIST_=(100 110 120 140 150 330 400 410 420 430 440 450 460)

> "$_LOG_"

for V in "${_VERSION_LIST_[@]}"; do

    echo "$V" >> "$_LOG_"

    find "$1" -iname "*.vert" | while read file; do

        echo "$file" >> "$_LOG_"

        echo "$V" "$file"

        echo "#version                        $V" >  temp
        echo "#define _CORE_VERTEX_SHADER_     1" >> temp
        echo "#define _CORE_QUALITY_           2" >> temp
        echo "#define CORE_NUM_TEXTURES_2D     4" >> temp
        echo "#define CORE_NUM_TEXTURES_SHADOW 1" >> temp
        echo "#define CORE_NUM_LIGHTS          4" >> temp
        echo "#define CORE_NUM_OUTPUTS         4" >> temp

        cat temp "$_GLOBAL_" "$_CUSTOM_" "$file" > temp.vert
        $_EXECUTABLE_ $_PARAMETERS_ temp.vert >> "$_LOG_"

    done

    find "$1" -iname "*.frag" | while read file; do

        echo "$file" >> "$_LOG_"

        echo "$V" "$file"

        echo "#version                        $V" >  temp
        echo "#define _CORE_FRAGMENT_SHADER_   1" >> temp
        echo "#define _CORE_QUALITY_           2" >> temp
        echo "#define CORE_NUM_TEXTURES_2D     4" >> temp
        echo "#define CORE_NUM_TEXTURES_SHADOW 1" >> temp
        echo "#define CORE_NUM_LIGHTS          4" >> temp
        echo "#define CORE_NUM_OUTPUTS         4" >> temp

        cat temp "$_GLOBAL_" "$_CUSTOM_" "$file" > temp.frag
        $_EXECUTABLE_ $_PARAMETERS_ temp.frag >> "$_LOG_"

    done

done

rm temp
rm temp.vert
rm temp.frag

xdg-open "$_LOG_"