cmake_minimum_required(VERSION 3.16)
set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)


project(CoreEngine VERSION 0.1.0)
add_library(CoreEngine STATIC)


set_target_properties(
    CoreEngine PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 17
    UNITY_BUILD ON
    DEBUG_POSTFIX _debug
    ARCHIVE_OUTPUT_DIRECTORY ${BASE_DIRECTORY}/output/linux/x64/
)


target_sources(
    CoreEngine PRIVATE
    ${BASE_DIRECTORY}/source/additional/linux/main.c
    ${BASE_DIRECTORY}/source/additional/coreCPUID.cpp
    ${BASE_DIRECTORY}/source/additional/coreGL.cpp
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


target_compile_options(
    CoreEngine PUBLIC
    -msse3 -mf16c -mxsave -Wall -Wextra -pedantic ${SANITIZER_OPTIONS}
    $<IF:$<CONFIG:Debug>, -O0 -glldb, -O3 -flto=thin -fwhole-program-vtables -ffinite-math-only -fno-rtti -fno-exceptions -Werror>
)


target_link_directories(
    CoreEngine INTERFACE
    ${BASE_DIRECTORY}/libraries/bin/linux/x64
)


target_link_libraries(
    CoreEngine INTERFACE
    -lX11 -lGL
    -l:libSDL2.so.0 -l:libSDL2_ttf.so.0 -l:libSDL2_image.so.0 -l:libGLEW.so.2 -l:libopenal.so.1 -l:libogg.so.0 -l:libvorbis.so.0 -l:libvorbisfile.so.3 -l:libzstd.so.1
)


target_link_options(
    CoreEngine INTERFACE
    -rdynamic -fuse-ld=lld ${SANITIZER_OPTIONS}
    $<IF:$<CONFIG:Debug>, , >
)