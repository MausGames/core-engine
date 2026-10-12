#!/bin/bash

# select target architecture
if [ "$(uname -m)" = "aarch64" ]; then
  cd bin/linux_arm_64
else
  cd bin/linux_x86_64
fi

# add library path
export LD_LIBRARY_PATH=".:$LD_LIBRARY_PATH"

# use Wayland/EGL instead of X11/GLX (or vice versa)
# export SDL_VIDEO_DRIVER="wayland"
# export SDL_VIDEO_DRIVER="x11"

# use Vulkan (Mesa Zink) instead of OpenGL
# export MESA_LOADER_DRIVER_OVERRIDE="zink"

# launch with gamemode (if available)
if type gamemoderun &> /dev/null; then
    gamemoderun ./CoreApp.elf "$@"
else
    ./CoreApp.elf "$@"
fi