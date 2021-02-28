cmake_minimum_required(VERSION 3.16)
set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)


project(CoreApp VERSION 0.0.1)
add_subdirectory(${BASE_DIRECTORY}/../CoreEngine/projects/cmake CoreEngine)
add_executable(CoreApp)


set_target_properties(
    CoreApp PROPERTIES
    C_STANDARD 11
    CXX_STANDARD 20
    DEBUG_POSTFIX _debug
    SUFFIX .elf
    RUNTIME_OUTPUT_DIRECTORY ${BASE_DIRECTORY}/executable/bin/linux/x64/
)


target_sources(
    CoreApp PRIVATE
    ${BASE_DIRECTORY}/source/application.cpp
    ${BASE_DIRECTORY}/source/main.cpp
)


target_include_directories(
    CoreApp PRIVATE
    ${BASE_DIRECTORY}/source
)


target_precompile_headers(
   CoreApp PRIVATE
   $<$<COMPILE_LANGUAGE:CXX>: ${BASE_DIRECTORY}/source/main.h>
)


target_link_libraries(
    CoreApp PRIVATE
    CoreEngine
)