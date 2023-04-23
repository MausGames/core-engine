#!/bin/bash

objcopy --only-keep-debug "$1" "$1.dbg"
objcopy --strip-unneeded "$1"
objcopy --add-gnu-debuglink="$1.dbg" "$1"