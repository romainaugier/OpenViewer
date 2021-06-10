# GLOBAL #

cmake_minimum_required(VERSION 3.15)

# Vcpkg
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "CMAKE Toolchain" FORCE)

# Linux std lib
if(UNIX)
	set(CMAKE_CXX_STANDARD_LIBRARIES "-ldl")
endif()

if(WIN32)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL 4)
		set(VCPKG_TARGET_TRIPLET "x86-windows" CACHE STRING "VCPKG Triplet" FORCE)
	elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
		set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "VCPKG Triplet" FORCE)
	endif()
elseif(UNIX)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL 4)
		message(FATAL_ERROR "VCPKG : linux:x86 triplet is not supported yet")
		# set(VCPKG_TARGET_TRIPLET "x86-linux" CACHE STRING "VCPKG Triplet" FORCE)
	elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
		set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "VCPKG Triplet" FORCE)
	endif()
endif()

# Project
project(OpenViewer)

# cpp
set(CMAKE_CXX_STANDARD 17)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(MSVC_COMPILER_FLAGS "/openmp:experimental /fp:fast /Ox /Oy /GT /GL /Oi /arch:AVX2 /Zi /Gm- /Zc:inline")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "${MSVC_COMPILER_FLAGS}")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options(-fopenmp -O3 -msse)
	add_link_options(-fopenmp)
else()
	message(FATAL_ERROR "Your compiler is not supported yet, CMake will exit.")
endif()

# OpenGL
include_directories(dependencies/glfw/include)

# OpenImageIO
find_package(OpenImageIO REQUIRED)

# As openimageio include variable works on window and not on linux, we need to use a hack to find
# the vcpkg include directory
if(WIN32)
	include_directories(${OPENIMAGEIO_INCLUDES})
elseif(UNIX)
	# Vcpkg include dir
	find_path(VCPKG_INCLUDE_DIR zlib.h)
	include_directories(${VCPKG_INCLUDE_DIR})
endif()

# OpenColorIO
find_package(OpenColorIO REQUIRED)

add_subdirectory(dependencies)
include_directories(dependencies)

include_directories(imgui)
add_subdirectory(imgui)

include_directories(src)
add_subdirectory(src)
