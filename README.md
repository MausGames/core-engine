![Core Engine](http://www.maus-games.at/site/images/core/core_header_cut.png)

# Description

The Core Engine is a **C++ framework** based on the [SDL library][10] for **Windows** and **Linux** games, currently migrated and re-written from old code.  
This project will take some time to reach a reasonable state, but it's **constantly updated and improved**.

The overall focus lies on a **supportive coding framework for fast and simple development** of multimedia applications and games.  
Some of the features already implemented are:

- Interfaces for System (Window, Events, Time), Graphics (Camera, View Frustum), Audio and Input
- Asynchronous Resource Management, Textures and Sounds
- SIMD Math (Basics, Vector, Matrix), Utilities (Data, Random, Splines), Threads and Timer
- HTML Log and Configuration File

If you have any suggestions or problems, feel free to contact me or simply create an issue on Bitbucket.  

# Libraries
- [SDL][10], [SDL_image][11] (jpeg, png, zlib), [SDL_ttf][12] (freetype)
- [OpenGL][13], [GLEW][14]
- [OpenAL][15], [Ogg Vorbis][16]


[10]: http://www.libsdl.org/
[11]: http://www.libsdl.org/projects/SDL_image/
[12]: http://www.libsdl.org/projects/SDL_ttf/
[13]: http://www.opengl.org/
[14]: http://glew.sourceforge.net/
[15]: http://www.openal-soft.org/
[16]: http://www.xiph.org/
[17]: http://curl.haxx.se/