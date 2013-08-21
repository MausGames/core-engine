![Core Engine](http://www.maus-games.at/site/images/core/core_header_cut.png)

# Description

The Core Engine is a **C++ framework** based on the [SDL library][10] for **Windows** and **Linux** games, currently migrated and re-written from old code.  
This project will take some time reaching a reasonable state, but it's **constantly updated and improved**.

The overall focus lies on a **supportive coding framework for very fast and simple development** of multimedia applications and games.  
Some of the features already implemented are:

- Interfaces for System (Window, Events, Time), Graphics (Camera, View Frustum), Audio and Input
- Asynchronous Resource Management and Textures
- SIMD Math (Basics, Vector, Matrix), Utilities (Data, Random, Splines) and Threads
- HTML Logs and Configuration Files

If you have any suggestions or problems, feel free to contact me or simply create an issue on Bitbucket.  
*(currently the framework works only with Visual C++)*

# Libraries
- [SDL][10], [SDL_image][11] (jpeg, png, zlib), [SDL_ttf][12] (freetype)
- [OpenGL][13], [GLEW][14]
- [OpenAL][15] (alut)
- [Ogg Vorbis][16]
- [cURL][17]


[10]: http://www.libsdl.org/
[11]: http://www.libsdl.org/projects/SDL_image/
[12]: http://www.libsdl.org/projects/SDL_ttf/
[13]: http://www.opengl.org/
[14]: http://glew.sourceforge.net/
[15]: http://connect.creativelabs.com/openal/
[16]: http://www.xiph.org/
[17]: http://curl.haxx.se/