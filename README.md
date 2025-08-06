![header](material/core_header.png)

# Core Engine

[![software license: zlib](material/readme/badge_license_software.svg)](LICENSE.txt)
[![content license: CC BY](material/readme/badge_license_content.svg)](https://creativecommons.org/licenses/by/4.0/)

The Core Engine is an extensive C++ framework based on the [SDL](https://libsdl.org/) library for **Windows, Linux, macOS and Browser games** and compatible with **OpenGL (2.0 - 4.6), OpenGL ES (2.0 - 3.2), WebGL (1.0 - 2.0)** and [ANGLE](https://angleproject.org/).

It is **designed to be supportive, versatile and powerful**, and to dynamically use efficient fallback methods where specific system features are not available.

## Features

- **Architecture**
  - **Unconventional but Efficient:** no default entity-component system, no default scene-graph, no default renderer, no redundant features, no redundant file formats, anything can be added on top
  - **Dual-Threaded Low-Latency Architecture:** main-thread and support-thread, sequential execution of move and render
  - **Strict Separation of Engine and Game:** even possible to reset the engine at any time or restart the game without restarting the process
- **System**
  - **Windows and Events:** resizable window mode, exclusive fullscreen mode, window configuration (icon, title), focus loss detection
  - **Displays:** resolutions, refresh rates, hot plugging
  - **Threads:** precise update rate, recursive work queue
  - **Time:** delta smoothing, speed configuration, simple timers (single float), complex timers (ticks/loops, state)
- **Graphics**
  - **OpenGL, OpenGL ES, WebGL:** extension-based feature selection, forward-compatibility with tons of fallbacks, state caching, explicit synchronization, ANGLE support (DirectX, Vulkan, Metal)
  - **Render State Handling:** camera and view frustum, lighting (simple), stencil testing, scissor testing, conservative rasterization, canonical aspect ratio (for sizing viewport and UI)
  - **Frame Buffers:** multi output-color support, invalidation, MSAA, [EQAA](https://www.pcgamingwiki.com/wiki/Glossary:Anti-aliasing_(AA)) (AMD), [CSAA](https://www.pcgamingwiki.com/wiki/Glossary:Anti-aliasing_(AA)) (Nvidia), [CMAA](https://www.pcgamingwiki.com/wiki/Glossary:Anti-aliasing_(AA)) (Intel)
  - **Data Buffers:** persistent mapping, multi-buffering, explicit orphaning, vertex format spec configuration (interleaved or separate)
  - **Models:** custom model format (for now), pre and post-transform vertex cache optimization, attribute packing, acceleration structure generation
  - **Textures:** PNG (flexible), WebP (fast), animated WebP, pixel-format conversion, block compression (DXT, BC7), mipmap generation, async upload, volume textures (simple), cube textures (simple)
  - **Shaders:** support for various GLSL shader types (vertex, tesselation, geometry, fragment, compute), parallel compilation, shader caching, [driver blob caching](https://registry.khronos.org/EGL/extensions/ANDROID/EGL_ANDROID_blob_cache.txt) (EGL), include system, compatibility layer, utility library, dynamic code injection, error and stats logging
  - **Fonts:** hinting, kerning, text and glyph layouting
  - **Async Screenshots**
- **Audio**
  - **OpenAL:** spatial mixing, state caching, operation batching, audio modes (mono, speakers, headphones), [HRTF](https://en.wikipedia.org/wiki/Head-related_transfer_function), resampler handling, device change and error recovery
  - **Music Files:** [Opus](https://opus-codec.org/), Vorbis (deprecated), threaded streaming, seamless transitioning and looping, music player system, pitch/speed controls
  - **Sound Effects:** Opus, Wave, [A-law](https://en.wikipedia.org/wiki/A-law_algorithm) and [μ-law](https://en.wikipedia.org/wiki/%CE%9C-law_algorithm) memory compression, sound categorization, pause and resume
- **Input**
  - **Mouse:** cursor graphics, visibility, keyboard and gamepad controls
  - **Keyboard:** [IME](https://en.wikipedia.org/wiki/Input_method) support, screen keyboard support
  - **Joysticks/Gamepads:** aggregation, hot plugging, rumble, led colors, both location and label-specific button testing
  - **Touch:** for mobile and desktop (including Steam Deck)
- **Debug**
  - **HTML Log:** colorful, thread-safe, with extensive system and resource logging
  - **CPU and GPU Performance Monitoring:** total and scope timings, total memory
  - **Pipeline Statistics:** vertex and fragment shader stats, culling stats, draw call and instance numbers
  - **Realtime Value and Texture Inspection**
  - **GPU Object Labels:** for driver logging and [RenderDoc](https://renderdoc.org/)
  - **Debug Input:** show debug overlay, frame stepping, explicit hot reloading
  - **Debug Visualizers:** [Natvis](https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects), [LLDB](https://lldb.llvm.org/use/variable.html)
  - **Crash Dumps:** Windows (full), Linux (custom)
  - **Pedantic Checking:** assertions everywhere, code sanitizer support (undefined behaviour, address, leak), floating-point exceptions
- **Platform**
  - **Dynamic Backend Loading:** select and use backend without static linking, Steamworks, Epic Online Services, Discord (only rich presence)
  - **User Management:** user name and ID, preferred language, ownership and connection check, per-user save folder (cloud compatible)
  - **Achievements and Stats:** regular and progressable achievements
  - **Leaderboards:** global and friend leaderboards, user score and rank, score paging, append extra data, attach uploaded files
  - **Rich Presence:** localization, basic [Steam Timeline](https://partner.steamgames.com/doc/features/timeline) support
  - **File Sharing:** public upload and download, download progress
- **Resource Management**
  - **Resource Handles:** reference counting, deferred unloading, indices instead of pointers, custom configuration and permutation
  - **Asynchronous and Parallel Loading**
  - **Packaging and Mounting**
  - **Dynamic Indirections/Proxies**
  - **Hot Reloading**
- **Memory Management**
  - **Memory Pools:** batch allocations, address sorting, memory arenas/heaps
  - **Static Allocations:** dynamically construct objects in predefined memory buffers without pointer-dereferencing on access
  - **Special Allocations:** zeroed, aligned, dynamic (realloc), temporary (very fast)
- **Configuration**
  - **Simple and Fast INI Files**
  - **Command Line Handling**
  - **Environment Variable Handling**
  - **Project Specific Settings**
- **Localization**
  - **Language Files:** simple key-value format, custom properties, value-placeholder assembly
  - **Realtime Language Switching**
  - **UTF-8 Support**
  - **Hot Reloading**
- **Game Objects**
  - **3D Objects:** non-uniform scaling, two-vector rotation (quaternion-based)
  - **2D Objects:** screen layouting (anchor/center, alignment), depth ordering
  - **Particle System:** custom animation and interaction handling, frame-rate independent spawning
  - **Precise Collision Detection:** 3-level cluster hierarchy, down to triangle-triangle and ray-triangle testing, intersection points, collision-pair tracking, separation between visual geometry and collision geometry (for simplification)
  - **Instancing/Batching**
- **Menu Objects**
  - **Labels:** outlining, automatic resolution-dependent resizing, pixel aligned rendering, fast and lightweight
  - **Richtext:** same as labels but heavier, inline formatting, per-character animations, automatic line wrapping
  - **Buttons**
  - **Check Boxes**
  - **Selection Boxes**
  - **Text Input Boxes:** input conceiling (for passwords)
  - **View/Scroll Boxes:** box culling
- **Math**
  - **Vectors:** 2D, 3D, 4D/quaterions, tons of utilities
  - **Matrices:** 2x2, 3x3, 4x4, other variants, also tons of utilities
  - **Color Space Conversion**: RGB, HSV, YIQ, YUV, YCbCr, Luminance
  - **Cubic Hermite Splines**: calculate position and direction, control tangents and acceleration, with refinement step
  - **Points:** with variable type and dimension
  - **Interpolation and Stepping**
  - **Rounding and Aligning**
  - **Value Packing:** half-float, various GPU formats, other general-purpose formats
  - **Bit and Byte Operations:** counting, rotating, reversing, converting
- **Utilities**
  - **Containers:** list (contiguous), set (contiguous, unsorted/sorted), map (contiguous, unsorted/sorted, string-key optimization, access caching)
  - **Random Numbers:** [PCG-XSH-RR 64/32](https://www.pcg-random.org/), individual state objects
  - **Compression:** [Zstandard](https://facebook.github.io/zstd/), also simple key-based in-place scrambling
  - **Hashing:** [xxHash](https://xxhash.com/) (primary), [Murmur 2+3](https://en.wikipedia.org/wiki/MurmurHash), [FNV-1a](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function), [CRC-32](https://en.wikipedia.org/wiki/Cyclic_redundancy_check), string hashing, compile-time hashing, hash mixing
  - **String Functions:** transformations, tokenization, safe string copy, string building, fast value/string conversions, path extractions, and a lot more
  - **Time Functions:** local and UTC, formatting
  - **Scopes and Smart Pointers**
  - **File Handling:** chunk-based archive container class, safe file writing (temp + move), various file and directory operations, recursive directory creation and enumeration, long path awareness (Windows)
  - **Process Handling:** application paths, application stats (CPU and GPU memory), dynamic library loading
  - **Operating System:** system paths, system stats (CPU and GPU memory, disk space), environment and hardware detections
  - **Synchronization**: recursive and non-recursive locks (spin-locks, mutexes), atomics
  - **Inline Text Formatting**: create formatted text without buffers or allocations
  - **Inline Preprocessor Testing**: check for any preprocessor symbol like a regular boolean
  - **... and everything from SDL**
- **Tools**
  - **Launcher:** Windows (application, script), Linux (script), Browser (page), for custom pre-launch processing
  - **Project Template:** default resources, update scripts
  - **Blender Export**
  - **Texture Optimization and Conversion**
  - **Shader Reference Validation**
  - **Automatic Code Signing (Windows)**
  - **Debug Data Separation (Linux)**
- **Building**
  - **Project Files:** Visual Studio (Windows), CMake/CLion (Linux, [Emscripten](https://emscripten.org/)), Xcode (macOS)
  - **Compiler Support:** MSVC, Clang with [Mold](https://github.com/rui314/mold), GCC (unmaintained), usually only the newest versions
  - **Architecture Support:** x86 (SSE), ARM (NEON), WebAssembly, both 32-bit and 64-bit
  - **Eclectic C++:** mix of modern features and traditional coding style
  - **Optimized for Size and Precision:** no fast-math allowed
  - **Native Wayland Support (Linux)**
  - **Native Apple Silicon Support (macOS)**
  - **Private Nintendo Switch™ Support (not open source)**
  - **Own Dependencies with Custom Modifications:** [GitHub](https://github.com/MausGames/core-library), [GitLab](https://gitlab.com/MausGames/core-library), no container or old distribution required for Linux builds

## Notes

- Please understand that pull requests are currently not accepted, as this engine is mainly a public personal project and learning experience.
- Some code changes are not yet available as they are still work-in-progress or confidental. There is no guarantee that everything will always work out-of-the-box on other peoples machines.
- Pre-built dependencies can be downloaded with the **complete_repository.bat/.sh** script or manually built from the **Core Library** repository ([GitHub](https://github.com/MausGames/core-library), [GitLab](https://gitlab.com/MausGames/core-library)).
- If you have any questions, please feel free to open an issue or [contact me](https://www.maus-games.at/) directly.

## Projects

| Name | Description | Links |
| --- | --- | --- |
| [Eigengrau](https://store.steampowered.com/app/1624320/Eigengrau/) | Colorful shoot'em up with varied and diverse gameplay | [GitHub](https://github.com/MausGames/project-one), [GitLab](https://gitlab.com/MausGames/project-one) |
| [Disco Rock](https://gamejolt.com/games/disco-rock/18996/) | Very simple endless-runner inspired by the 70s disco era | [GitHub](https://github.com/MausGames/disco-rock), [GitLab](https://gitlab.com/MausGames/disco-rock) |
| Flip It | Competitive territory game made on [Mi'pu'mi][1] day | [GitHub](https://github.com/MausGames/flip-it), [GitLab](https://gitlab.com/MausGames/flip-it) |
| Party Raider | Multiplayer platform shooter made on [Mi'pu'mi][1] day | [GitHub](https://github.com/MausGames/party-raider), [GitLab](https://gitlab.com/MausGames/party-raider) |
| [Endless Journey](https://mausgames.itch.io/endless-journey) | Spiritual arcade climber made on [Mi'pu'mi][1] day | [GitHub](https://github.com/MausGames/endless-journey), [GitLab](https://gitlab.com/MausGames/endless-journey) |
| [Last Fall](https://mausgames.itch.io/last-fall) | Tricky arcade puzzler made for [Ludum Dare 49](https://ldjam.com/events/ludum-dare/49/last-fall) | [GitHub](https://github.com/MausGames/last-fall), [GitLab](https://gitlab.com/MausGames/last-fall) |
| [Ink Jump](https://mausgames.itch.io/ink-jump) | Reflective precision platformer made on [Mi'pu'mi][1] day  | [GitHub](https://github.com/MausGames/ink-jump), [GitLab](https://gitlab.com/MausGames/ink-jump) |

<table>
    <tr>
        <td><a href="material/screenshots/p1_screen_021.jpg?raw=true"><img src="material/screenshots/p1_screen_021t.jpg"></a></td>
        <td><a href="material/screenshots/laf_screen_006.jpg?raw=true"><img src="material/screenshots/laf_screen_006t.jpg"></a></td>
        <td><a href="material/screenshots/inj_screen_001.jpg?raw=true"><img src="material/screenshots/inj_screen_001t.jpg"></a></td>
    </tr>
</table>

[1]: https://mipumi.com