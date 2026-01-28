cmake_minimum_required(VERSION 3.20)

project(renderer
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)

FetchContent_MakeAvailable(glm)

add_executable(renderer
    src/main.cpp
    src/application.cpp
    src/world.cpp
    src/camera.cpp
    src/picture.cpp
    src/renderer.cpp
    src/image_io.cpp
    src/clip.cpp
    src/rasterizer.cpp
)

target_link_libraries(renderer PRIVATE glm::glm)
