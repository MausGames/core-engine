#!/bin/bash

_EXECUTABLE_="./base/optipng.elf"
_PARAMETERS_="-strip all -nx -o7"

$_EXECUTABLE_ $_PARAMETERS_ "$1"