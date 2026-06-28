function(core_add_executable TARGET)

    add_subdirectory(${ENGINE_DIRECTORY}/projects/cmake CoreEngine)
    add_executable(${TARGET})

    set_target_properties(
        ${TARGET} PROPERTIES
        C_STANDARD 17
        CXX_STANDARD 23
        CXX_SCAN_FOR_MODULES OFF
        DEBUG_POSTFIX _debug
    )

    target_include_directories(
        ${TARGET} PRIVATE
        ${BASE_DIRECTORY}/source
    )

    target_precompile_headers(
       ${TARGET} PRIVATE
       $<$<COMPILE_LANGUAGE:CXX>: ${BASE_DIRECTORY}/source/main.h>
    )

    target_link_libraries(
        ${TARGET} PRIVATE
        CoreEngine
    )

    if(CORE_EMSCRIPTEN)

        set_target_properties(
            ${TARGET} PROPERTIES
            SUFFIX .html
            RUNTIME_OUTPUT_DIRECTORY ${BASE_DIRECTORY}/executable/bin/emscripten
        )

        if(NOT CORE_DEBUG)

            add_custom_command(
                TARGET ${TARGET} POST_BUILD VERBATIM
                COMMAND mv -f $<TARGET_FILE:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>/index.html
            )

        endif()

    else()

        set_target_properties(
            ${TARGET} PROPERTIES
            SUFFIX .elf
            RUNTIME_OUTPUT_DIRECTORY ${BASE_DIRECTORY}/executable/bin/linux_x86_64
        )

        if(NOT CORE_DEBUG)

            set_target_properties(
                ${TARGET} PROPERTIES
                BUILD_WITH_INSTALL_RPATH TRUE
                INSTALL_RPATH $ORIGIN
            )

            add_custom_command(
                TARGET ${TARGET} POST_BUILD VERBATIM
                COMMAND ${ENGINE_DIRECTORY}/tools/scripts/extract_symbols.sh $<TARGET_FILE:${TARGET}>
            )

        endif()

    endif()

endfunction()