#!/bin/bash
cd bin/linux_x86_64

# set library path
export LD_LIBRARY_PATH=".:$LD_LIBRARY_PATH"

# use Wayland/EGL instead of X11/GLX
# export SDL_VIDEODRIVER="wayland"

# use mimalloc allocator (if available)
export LD_PRELOAD="libmimalloc.so.2:$LD_PRELOAD"

# launch with gamemode (if available)
if type gamemoderun &> /dev/null; then
    gamemoderun ./CoreApp.elf "$@"
else
    ./CoreApp.elf "$@"
fi