# Core Engine

[![software license: zlib](material/readme/badge_license_software.svg)](LICENSE.txt)
[![content license: CC BY](material/readme/badge_license_content.svg)](https://creativecommons.org/licenses/by/4.0/)

The Core Engine is an extensive C++ framework based on the [SDL](https://libsdl.org/) library for **Windows, Linux, macOS and Browser games** and compatible with **OpenGL (2.0 - 4.6), OpenGL ES (2.0 - 3.2), WebGL (1.0 - 2.0)** and [ANGLE](https://angleproject.org/).

It is **designed to be supportive, versatile and powerful**, and to dynamically use efficient fallback methods where specific system features are not available.

> - **System:** Hardware, Window and Event Handling, Timer, Threads
> - **Graphics:** Camera, View Frustum and Lighting Control, Frame Buffers, Data Buffers, Models (.md3, .md5, packed), Textures (.png, .webp, compressed, animated), Shaders (parallel compiled, cached), Fonts, Screenshots
> - **Audio:** Sound Effects (.wav, .opus, compressed), Music Files (.ogg, .opus, streamed), Music Player (seamless, threaded)
> - **Input:** Mouse, Keyboard, Joysticks/Gamepads, Touch, Rumble
> - **Debug:** CPU and GPU Performance Monitor, Run-Time Value and Texture Inspection, Pipeline Statistics, Debug Visualizers (Natvis, LLDB), Crash Dumps
> - **Platform:** Achievements, Stats, Leaderboards, Data Sharing, Rich Presence, User Management, Dynamic Backend Loading (Steamworks, EOS)
> - Asynchronous Resource Management (Hot-Reloading, Indirections/Proxies), Memory Management (Global Sharing, Pool Allocations, Alignment)
> - File Handling (Packaging, Mounting, Compression, Safe Writing), HTML Log, Configuration and Language Files
> - Compile-Time Hashing, Random Numbers, Specialized Containers (List, Set, Map), General Utilities (String, Time, OS, etc.)
> - Vectors (2d, 3d, 4d/Quaternions, Colors), Matrices (2x2, 3x3, 4x4, other), Splines
> - Game Objects (2d, 3d), Particle System, Batching, Precise Collision Detection
> - Menu Objects (Labels (simple, rich), Buttons, Check Boxes, Selection Boxes, Text Boxes, View/Scroll Boxes)
> - Tools (Blender Export, Shader Validation, Texture Optimization), Project Template, Launcher (Windows, Linux, Browser)

Please understand that pull requests are currently not accepted, as this is mainly a (public) personal project and learning experience. There is no guarantee that everything will always work out-of-the-box on other peoples machines.

Pre-built dependencies can be downloaded with **tools/complete_repository.bat/.sh** or manually built from the **Core Library** ([GitHub](https://github.com/MausGames/core-library), [GitLab](https://gitlab.com/MausGames/core-library)).

#

| Name | Description | Links |
| --- | --- | --- |
| [Eigengrau](https://store.steampowered.com/app/1624320/Eigengrau/) | Colorful shoot'em up with varied and diverse gameplay | [GitHub](https://github.com/MausGames/project-one), [GitLab](https://gitlab.com/MausGames/project-one) |
| [Disco Rock](https://gamejolt.com/games/disco-rock/18996/) | Very simple endless-runner inspired by the 70s disco era | [GitHub](https://github.com/MausGames/disco-rock), [GitLab](https://gitlab.com/MausGames/disco-rock) |
| Flip It | Competitive territory game made on [Mi'pu'mi][1] day | [GitHub](https://github.com/MausGames/flip-it), [GitLab](https://gitlab.com/MausGames/flip-it) |
| Party Raider | Multiplayer platform shooter made on [Mi'pu'mi][1] day | [GitHub](https://github.com/MausGames/party-raider), [GitLab](https://gitlab.com/MausGames/party-raider) |
| [Endless Journey](https://mausgames.itch.io/endless-journey) | Spiritual arcade climber made on [Mi'pu'mi][1] day | [GitHub](https://github.com/MausGames/endless-journey), [GitLab](https://gitlab.com/MausGames/endless-journey) |
| [Last Fall](https://mausgames.itch.io/last-fall) | Tricky arcade puzzler made for [Ludum Dare 49](https://ldjam.com/events/ludum-dare/49/last-fall) | [GitHub](https://github.com/MausGames/last-fall), [GitLab](https://gitlab.com/MausGames/last-fall) |
| [Ink Jump](https://mausgames.itch.io/ink-jump) | Reflective precision platformer made on [Mi'pu'mi][1] day  | [GitHub](https://github.com/MausGames/ink-jump), [GitLab](https://gitlab.com/MausGames/ink-jump) |

#

<table>
    <tr>
        <td><a href="material/screenshots/p1_screen_021.jpg?raw=true"><img src="material/screenshots/p1_screen_021t.jpg"></a></td>
        <td><a href="material/screenshots/laf_screen_006.jpg?raw=true"><img src="material/screenshots/laf_screen_006t.jpg"></a></td>
        <td><a href="material/screenshots/inj_screen_001.jpg?raw=true"><img src="material/screenshots/inj_screen_001t.jpg"></a></td>
    </tr>
</table>

[1]: https://mipumi.com