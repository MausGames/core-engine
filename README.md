![Core Engine](http://www.maus-games.at/site/images/core/core_header_cut.png)

# Description
The Core Engine is a C++ framework based on the SDL library for **Windows, Linux and Android games.** It's currently under initial development and gets re-written from old code, some of the features already implemented are listed below.

This project is focused on creating a **supportive framework for fast and simple development** of multimedia applications and games, with the personal intention to learn new stuff and to have a structured coding base (core) for other projects.

If you have any suggestions or problems, feel free to contact me or simply create an issue on Bitbucket.

> - **System:** Window and Event Handling, Timer, Threads 
> - **Graphics:** Camera and View Frustum Control, Ambient and Lighting Control, Frame Buffers, Models (.md5), Textures, Shaders, Fonts
> - **Audio:** Sound Effects (.wav), Music Files (.ogg), Music Player
> - **Input:** Mouse, Keyboard, Joysticks/Gamepads 
> - **Math:** SIMD Implementations, Vectors (2d, 3d, 4d/Quaternions), Matrices (3x3, 4x4), Splines
> - **Data:** String/Stream Manipulation, Random Numbers, Fast-Lookup Container
> - Asynchronous Resource Management
> - Memory Management (currently only Memory Sharing)
> - Basic Filesystem, HTML Log, Configuration File
> - Game Objects (2d, 3d)
> - Menu Objects (Labels, Buttons, Check Boxes, Selection Boxes, Text Boxes)


# Download

Documentation and separate download **will be available with the first release** (v0.1.0), until then the current snapshot of the framework can be retrieved from here.

You also need the latest Android [SDK][5] and [NDK][6], the [JDK][7] and [Ant][8] if you want to develop for this platform.

> **Supported compilers:** GCC/MinGW 4.8+, MSVC 12.0+, Clang 3.3+  
> Documentation for the framework is currently not available


# Libraries
External libraries used by the framework

- [SDL][10], [SDL_image][11] (jpeg, png, zlib), [SDL_ttf][12] (freetype)
- [OpenGL][13], [GLEW][14]
- [OpenAL][15], [Ogg Vorbis][16]

Specific libraries used for Android development

- [OpenGL ES 2.0][17]
- [OpenAL with Android backend][18]


[5]:  http://developer.android.com/sdk/
[6]:  http://developer.android.com/tools/sdk/ndk/
[7]:  http://www.oracle.com/technetwork/java/javase/downloads/
[8]:  http://ant.apache.org/
[10]: http://www.libsdl.org/
[11]: http://www.libsdl.org/projects/SDL_image/
[12]: http://www.libsdl.org/projects/SDL_ttf/
[13]: http://www.khronos.org/opengl/
[14]: http://glew.sourceforge.net/
[15]: http://www.openal-soft.org/
[16]: http://www.xiph.org/
[17]: http://www.khronos.org/opengles/
[18]: http://repo.or.cz/w/openal-soft/android.git