cmake_minimum_required(VERSION 3.21)
set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)


project(CoreEngine VERSION 0.1.0)
add_library(CoreEngine STATIC)


set_target_properties(
    CoreEngine PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 20
    DEBUG_POSTFIX _debug
    UNITY_BUILD ON
)


target_sources(
    CoreEngine PRIVATE
    ${BASE_DIRECTORY}/source/additional/coreCPUID.cpp
    ${BASE_DIRECTORY}/source/components/audio/CoreAudio.cpp
    ${BASE_DIRECTORY}/source/components/audio/coreMusic.cpp
    ${BASE_DIRECTORY}/source/components/audio/coreSound.cpp
    ${BASE_DIRECTORY}/source/components/debug/CoreDebug.cpp
    ${BASE_DIRECTORY}/source/components/graphics/coreDataBuffer.cpp
    ${BASE_DIRECTORY}/source/components/graphics/coreFont.cpp
    ${BASE_DIRECTORY}/source/components/graphics/coreFrameBuffer.cpp
    ${BASE_DIRECTORY}/source/components/graphics/CoreGraphics.cpp
    ${BASE_DIRECTORY}/source/components/graphics/coreModel.cpp
    ${BASE_DIRECTORY}/source/components/graphics/coreShader.cpp
    ${BASE_DIRECTORY}/source/components/graphics/coreSync.cpp
    ${BASE_DIRECTORY}/source/components/graphics/coreTexture.cpp
    ${BASE_DIRECTORY}/source/components/input/CoreInput.cpp
    ${BASE_DIRECTORY}/source/components/platform/CorePlatform.cpp
    ${BASE_DIRECTORY}/source/components/system/CoreSystem.cpp
    ${BASE_DIRECTORY}/source/components/system/coreThread.cpp
    ${BASE_DIRECTORY}/source/components/system/coreTimer.cpp
    ${BASE_DIRECTORY}/source/manager/coreMemory.cpp
    ${BASE_DIRECTORY}/source/manager/coreObject.cpp
    ${BASE_DIRECTORY}/source/manager/coreResource.cpp
    ${BASE_DIRECTORY}/source/objects/game/coreObject2D.cpp
    ${BASE_DIRECTORY}/source/objects/game/coreObject3D.cpp
    ${BASE_DIRECTORY}/source/objects/game/coreParticle.cpp
    ${BASE_DIRECTORY}/source/objects/menu/coreButton.cpp
    ${BASE_DIRECTORY}/source/objects/menu/coreCheckBox.cpp
    ${BASE_DIRECTORY}/source/objects/menu/coreLabel.cpp
    ${BASE_DIRECTORY}/source/objects/menu/coreMenu.cpp
    ${BASE_DIRECTORY}/source/objects/menu/coreTextBox.cpp
    ${BASE_DIRECTORY}/source/utilities/data/coreData.cpp
    ${BASE_DIRECTORY}/source/utilities/file/coreArchive.cpp
    ${BASE_DIRECTORY}/source/utilities/file/coreConfig.cpp
    ${BASE_DIRECTORY}/source/utilities/file/coreLanguage.cpp
    ${BASE_DIRECTORY}/source/utilities/file/coreLog.cpp
    ${BASE_DIRECTORY}/source/Core.cpp
)


target_include_directories(
    CoreEngine PUBLIC
    ${BASE_DIRECTORY}/source
    ${BASE_DIRECTORY}/libraries/include
)


target_precompile_headers(
    CoreEngine PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>: ${BASE_DIRECTORY}/source/Core.h>
)


if(EMSCRIPTEN)


    set_target_properties(
        CoreEngine PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${BASE_DIRECTORY}/output/emscripten/
    )


    target_sources(
        CoreEngine PRIVATE
        ${BASE_DIRECTORY}/source/additional/emscripten/main.c
        ${BASE_DIRECTORY}/source/additional/coreGLES.cpp
    )


    target_compile_options(
        CoreEngine PUBLIC
        -pipe -Wall -Wextra -pedantic
        $<IF:$<CONFIG:Debug>, -O0 -g3, -Os -flto -fwhole-program-vtables -ffinite-math-only -fno-rtti -fno-exceptions -Werror>
    )


    target_link_options(
        CoreEngine INTERFACE
        -lopenal -lidbfs.js -sUSE_SDL=2 -sUSE_SDL_TTF=2 -sUSE_SDL_IMAGE=2 -sUSE_OGG=1 -sUSE_VORBIS=1 -sSDL2_IMAGE_FORMATS=png -sENVIRONMENT=web -sMAX_WEBGL_VERSION=2 -sALLOW_MEMORY_GROWTH=1
        --preload-file ../../../executable/data@/data --shell-file ${BASE_DIRECTORY}/source/additional/emscripten/shell.html --pre-js ${BASE_DIRECTORY}/source/additional/emscripten/pre.js
        $<IF:$<CONFIG:Debug>, -O0 -g3, -Os --closure 1>
    )


else()


    set_target_properties(
        CoreEngine PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${BASE_DIRECTORY}/output/linux_x86_64/
    )


    target_sources(
        CoreEngine PRIVATE
        ${BASE_DIRECTORY}/source/additional/linux/main.c
        ${BASE_DIRECTORY}/source/additional/coreGL.cpp
        ${BASE_DIRECTORY}/source/additional/coreGLEW.c
    )


    target_compile_options(
        CoreEngine PUBLIC
        -msse3 -mf16c -pipe -Wall -Wextra -pedantic ${SANITIZER_OPTIONS}
        $<IF:$<CONFIG:Debug>, -O0 -glldb, -Os -flto=thin -fwhole-program-vtables -ffinite-math-only -fno-rtti -fno-exceptions -Werror>
    )


    target_link_directories(
        CoreEngine INTERFACE
        ${BASE_DIRECTORY}/libraries/bin/linux_x86_64
    )


    target_link_libraries(
        CoreEngine INTERFACE
        -ldl -lpthread -lGL
        -l:libSDL2.so.0 -l:libSDL2_ttf.so.0 -l:libSDL2_image.so.0 -l:libopenal.so.1 -l:libogg.so.0 -l:libvorbis.so.0 -l:libvorbisfile.so.3 -l:libzstd.so.1
    )


    target_link_options(
        CoreEngine INTERFACE
        -fuse-ld=lld ${SANITIZER_OPTIONS}
        $<IF:$<CONFIG:Debug>, , -rdynamic -s>
    )


endif()