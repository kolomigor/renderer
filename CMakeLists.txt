cmake_minimum_required(VERSION 3.20)

project(renderer
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(CTest)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.32.10
)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

set(TINYOBJLOADER_BUILD_TEST_LOADER OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG v2.0.0rc13
)

FetchContent_MakeAvailable(glm SDL2 nlohmann_json tinyobjloader)

find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

if(TARGET SDL2::SDL2-static)
    set(SDL2_TARGET SDL2::SDL2-static)
elseif(TARGET SDL2::SDL2)
    set(SDL2_TARGET SDL2::SDL2)
else()
    message(FATAL_ERROR "SDL2 target was not created")
endif()

if(TARGET tinyobjloader)
    set(TINYOBJLOADER_TARGET tinyobjloader)
elseif(TARGET tinyobjloader::tinyobjloader)
    set(TINYOBJLOADER_TARGET tinyobjloader::tinyobjloader)
else()
    message(FATAL_ERROR "tinyobjloader target was not created")
endif()

add_executable(renderer
    src/main.cpp
    src/application.cpp
    src/world.cpp
    src/camera.cpp
    src/picture.cpp
    src/renderer.cpp
    src/image_io.cpp
    src/texture.cpp
    src/clip.cpp
    src/rasterizer.cpp
    src/except.cpp
    src/lights.cpp
    src/mesh.cpp
    src/scene.cpp
    src/window.cpp
)

target_link_libraries(renderer PRIVATE
    glm::glm
    nlohmann_json::nlohmann_json
    PNG::PNG
    JPEG::JPEG
    ${SDL2_TARGET}
    ${TINYOBJLOADER_TARGET}
)

add_executable(renderer_tests
    tests/renderer_tests.cpp
    src/camera.cpp
    src/clip.cpp
    src/lights.cpp
    src/picture.cpp
    src/rasterizer.cpp
    src/renderer.cpp
    src/texture.cpp
    src/world.cpp
)

target_include_directories(renderer_tests PRIVATE src)
target_link_libraries(renderer_tests PRIVATE glm::glm PNG::PNG JPEG::JPEG)

add_test(NAME renderer_tests COMMAND renderer_tests)
