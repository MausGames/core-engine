cmake_minimum_required(VERSION 3.16)
set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)


project(CoreEngine VERSION 0.0.9)
add_library(CoreEngine STATIC)


set_target_properties(
    CoreEngine PROPERTIES
    CXX_STANDARD 17
    UNITY_BUILD ON
    DEBUG_POSTFIX _debug
    ARCHIVE_OUTPUT_DIRECTORY ${BASE_DIRECTORY}/output/linux/x64/
)


target_sources(
    CoreEngine PRIVATE
    ${BASE_DIRECTORY}/libraries/include/SI/ConvertUTF.c
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
    -msse3 -mf16c -mxsave -Wall -Wextra -pedantic
    $<IF:$<CONFIG:Debug>, -O0 -glldb, -O3 -ffinite-math-only -fno-rtti -fno-exceptions -Werror>
)


target_link_directories(
    CoreEngine INTERFACE
    ${BASE_DIRECTORY}/libraries/bin/linux/x64
)


target_link_libraries(
    CoreEngine INTERFACE
    -lX11 -lGL
    -lSDL2 -lSDL2_ttf -lSDL2_image -lGLEW -lopenal -logg -lvorbis -lvorbisfile -lzstd
)


target_link_options(
    CoreEngine INTERFACE
    -rdynamic
    $<IF:$<CONFIG:Debug>, , >
)