cmake_minimum_required(VERSION 3.20)
project(Voxel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---------- Compiled libraries ----------
# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# GLEW
FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/nigels-com/glew.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glew)
set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "Do not build FreeGLUT demos" FORCE)
set(FREEGLUT_BUILD_TESTS OFF CACHE BOOL "Do not build FreeGLUT tests" FORCE)
# FreeGLUT
FetchContent_Declare(
    freeglut
    GIT_REPOSITORY https://github.com/FreeGLUTProject/freeglut.git
    GIT_TAG master
)
FetchContent_MakeAvailable(freeglut)

# ---------- Your project ----------
add_executable(Voxel
    Voxel/Voxel.cpp
    # Add other source files here
)

# Include directories
target_include_directories(Voxel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/external/PerlinNoise
    ${glfw_SOURCE_DIR}/include
    ${glew_SOURCE_DIR}/include
    ${freeglut_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(Voxel PRIVATE
    glfw
    glew_s
    freeglut
)

# OpenGL linking
if(WIN32)
    target_link_libraries(Voxel PRIVATE opengl32)
elseif(UNIX)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Voxel PRIVATE OpenGL::GL)
endif()