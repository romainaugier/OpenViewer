# GLOBAL #

cmake_minimum_required(VERSION 3.15)

# Vcpkg
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "CMAKE Toolchain" FORCE)

if(WIN32)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL 4)
		set(VCPKG_TARGET_TRIPLET "x86-windows" CACHE STRING "VCPKG Triplet" FORCE)
	elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
		set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "VCPKG Triplet" FORCE)
	endif()
elseif(UNIX)
	message("Linux is not supported yet")
endif()

# Project
project(OpenViewer)

# cpp
set(CMAKE_CXX_STANDARD 17)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# MSVC Flags
	set(MSVC_COMPILER_FLAGS "/openmp:experimental /fp:fast /Ox /Oy /GT /GL /Oi /arch:AVX2 /Zi /Gm- /Zc:inline")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "${MSVC_COMPILER_FLAGS}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
	message(FATAL_ERROR "Your compiler is not supported yet, CMake will exit.")
endif()

# OpenGL
include_directories(dependencies/glfw/include)

# OpenImageIO
find_package(OpenImageIO REQUIRED)
include_directories(${OPENIMAGEIO_INCLUDES})

message(${OPENIMAGEIO_INCLUDES})

include_directories(imgui)
add_subdirectory(imgui)

include_directories(src)
add_subdirectory(src)
