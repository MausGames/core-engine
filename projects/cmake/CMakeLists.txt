cmake_minimum_required(VERSION 3.25)

set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)

project(CoreEngine VERSION 1.0.0)
add_library(CoreEngine STATIC)

set_target_properties(
    CoreEngine PROPERTIES
    C_STANDARD 17
    CXX_STANDARD 23
    DEBUG_POSTFIX _debug
)

target_sources(
    CoreEngine PRIVATE
    ${BASE_DIRECTORY}/source/additional/coreCPUID.cpp
    ${BASE_DIRECTORY}/source/additional/coreImGui.cpp
    ${BASE_DIRECTORY}/source/components/audio/CoreAudio.cpp
    ${BASE_DIRECTORY}/source/components/audio/coreMusic.cpp
    ${BASE_DIRECTORY}/source/components/audio/coreSound.cpp
    ${BASE_DIRECTORY}/source/components/debug/CoreDebug.cpp
    ${BASE_DIRECTORY}/source/components/graphics/coreAnimation.cpp
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
    ${BASE_DIRECTORY}/source/objects/menu/coreRichText.cpp
    ${BASE_DIRECTORY}/source/objects/menu/coreTextBox.cpp
    ${BASE_DIRECTORY}/source/objects/menu/coreViewBox.cpp
    ${BASE_DIRECTORY}/source/utilities/data/hash/XXH.cpp
    ${BASE_DIRECTORY}/source/utilities/data/coreData.cpp
    ${BASE_DIRECTORY}/source/utilities/data/coreString.cpp
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

if(CORE_EMSCRIPTEN)

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
        -msse3 -pipe -Wall -Wextra -Wpedantic
        -matomics -mbulk-memory -mextended-const -mmultivalue -mmutable-globals -mnontrapping-fptoint -mreference-types -msign-ext -msimd128 -mtail-call
        $<IF:$<CONFIG:DEBUG>, -O0 -g3 -gsplit-dwarf -gz=zlib -ftrivial-auto-var-init=pattern, -Os -flto -fwhole-program-vtables -fvirtual-function-elimination -fforce-emit-vtables -fstrict-vtable-pointers -fno-strict-overflow -ffinite-math-only -fno-signed-zeros -fno-trapping-math -fno-math-errno -ffp-contract=off -fno-rtti -fno-exceptions -Werror>
    )

    target_link_directories(
        CoreEngine INTERFACE
        ${BASE_DIRECTORY}/libraries/bin/emscripten
    )

    target_link_libraries(
        CoreEngine INTERFACE
        -lopenal -lidbfs.js
        -l:libSDL3.a -l:libSDL3_ttf.a -l:libSDL3_image.a -l:libwebp.a -l:libwebpdemux.a -l:libzstd.a -l:libopusfile.a -l:libopus.a -l:libogg.a
    )

    target_link_options(
        CoreEngine INTERFACE
        -sENVIRONMENT=web -sMAX_WEBGL_VERSION=2 -sGL_ENABLE_GET_PROC_ADDRESS=1 -sWASM_BIGINT=1 -sMALLOC=mimalloc -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=1MB
        --preload-file ../../../executable/data_web@/data --shell-file ${BASE_DIRECTORY}/source/additional/emscripten/shell.html --pre-js ${BASE_DIRECTORY}/source/additional/emscripten/pre.js
        $<IF:$<CONFIG:DEBUG>, -O0 -g3 -sGL_ASSERTIONS=1, -Os -flto -sGL_TRACK_ERRORS=0 -sEVAL_CTORS=1 --closure 1>
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
        -msse3 -pipe -Wall -Wextra -Wpedantic
        $<IF:$<CONFIG:DEBUG>, -O0 -g3 -glldb -gsplit-dwarf -gz=zlib -ftrivial-auto-var-init=pattern, -Os -flto -fwhole-program-vtables -fvirtual-function-elimination -fforce-emit-vtables -fstrict-vtable-pointers -fno-strict-overflow -ffinite-math-only -fno-signed-zeros -fno-trapping-math -fno-math-errno -ffp-contract=off -fno-rtti -fno-exceptions -Werror>
    )

    target_link_directories(
        CoreEngine INTERFACE
        ${BASE_DIRECTORY}/libraries/bin/linux_x86_64
    )

    target_link_libraries(
        CoreEngine INTERFACE
        -ldl -lpthread -lGL
        -l:libSDL3.so -l:libSDL3_ttf.so -l:libSDL3_image.so -l:libopenal.so -l:libzstd.so -l:libopusfile.so
    )

    target_link_options(
        CoreEngine INTERFACE
        -fuse-ld=mold LINKER:--compress-debug-sections=zlib
        $<IF:$<CONFIG:DEBUG>, LINKER:--separate-debug-file, -flto -rdynamic LINKER:--gc-sections LINKER:--icf=all LINKER:--ignore-data-address-equality LINKER:--as-needed>
    )

    if(NOT CORE_DEBUG)

        set(LIBCXX_DIRECTORY ${BASE_DIRECTORY}/../CoreLibrary/output/libcxx)
        set(GLIBC_DIRECTORY ${BASE_DIRECTORY}/../CoreLibrary/output/glibc)

        target_sources(
            CoreEngine INTERFACE
            ${GLIBC_DIRECTORY}/lib/crt1.o
            ${GLIBC_DIRECTORY}/lib/crti.o
            ${GLIBC_DIRECTORY}/lib/crtn.o
        )

        target_include_directories(
            CoreEngine SYSTEM PUBLIC
            ${LIBCXX_DIRECTORY}/include/c++/v1
            ${GLIBC_DIRECTORY}/include
        )

        target_compile_options(
            CoreEngine PUBLIC
            -nostdinc++
        )

        target_link_directories(
            CoreEngine INTERFACE
            ${LIBCXX_DIRECTORY}/lib
            ${GLIBC_DIRECTORY}/lib
        )

        target_link_libraries(
            CoreEngine INTERFACE
            -lm -lc
            -l:libc++.so
        )

        target_link_options(
            CoreEngine INTERFACE
            -nostdlib -nostdlib++
        )

    endif()

endif()

if(NOT CORE_DEBUG)

    set_source_files_properties(
        ${BASE_DIRECTORY}/source/components/graphics/coreModel.cpp
        ${BASE_DIRECTORY}/source/components/graphics/coreTexture.cpp
        ${BASE_DIRECTORY}/source/manager/coreObject.cpp
        PROPERTIES
        COMPILE_FLAGS -O3
        SKIP_PRECOMPILE_HEADERS ON
    )

endif()

if(CORE_TEST)

    target_compile_options(
        CoreEngine PUBLIC
        -fsanitize=undefined,address,leak -fverify-intermediate-code
    )

    target_link_options(
        CoreEngine INTERFACE
        -fsanitize=undefined,address,leak
    )

endif()