# GLOBAL #

cmake_minimum_required(VERSION 3.15)

project(OpenViewer)

include_directories(dependencies/glfw/include)

# OpenImageIO
find_package(OpenImageIO REQUIRED)
include_directories(${OpenImageIO_INCLUDE_DIRS})

include_directories(imgui)
add_subdirectory(imgui)

add_subdirectory(src)