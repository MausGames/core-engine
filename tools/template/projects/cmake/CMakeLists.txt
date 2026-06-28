cmake_minimum_required(VERSION 4.2)

set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(ENGINE_DIRECTORY ${BASE_DIRECTORY}/../core-engine)

project(
    CoreApp
    VERSION 0.0.1
    DESCRIPTION ""
    HOMEPAGE_URL "https://www.maus-games.at"
    LANGUAGES C CXX
)

include(${ENGINE_DIRECTORY}/projects/cmake/shared.cmake)
core_add_executable(CoreApp)

target_sources(
    CoreApp PRIVATE
    ${BASE_DIRECTORY}/source/application.cpp
    ${BASE_DIRECTORY}/source/main.cpp
)