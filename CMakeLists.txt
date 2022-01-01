# GLOBAL #

cmake_minimum_required(VERSION 3.15)


# Vcpkg
if(DEFINED CMAKE_TOOLCHAIN_FILE)
	if(EXISTS "${CMAKE_TOOLCHAIN_FILE}")
		message(STATUS "Vcpkg has been found")
	else()
		message(FATAL_ERROR "Vcpkg can't be found, CMake will exit")
	endif()
else()
	if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
		message(STATUS "Vcpkg has been found")	
		set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "CMAKE Toolchain" FORCE)
	else()
		message(FATAL_ERROR "Vcpkg can't be found, CMake will exit")
	endif()
endif()

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
		message(FATAL_ERROR "VCPKG : linux:x86 triplet is not supported yet, CMake will exit.")
		# set(VCPKG_TARGET_TRIPLET "x86-linux" CACHE STRING "VCPKG Triplet" FORCE)
	elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL 8)
		set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "VCPKG Triplet" FORCE)
	endif()
endif()

# Project
project(OpenViewer)

# cpp
set(CMAKE_CXX_STANDARD 17)

# to correct the bug appearing in half.h
if(WIN32)
	add_definitions(-DOPENEXR_DLL)
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_compile_definitions(-D_CRT_SECURE_NO_WARNINGS -DOPENEXR_DLL)
	set(MSVC_COMPILER_FLAGS "/fp:fast /Ox /Os /Oy /GT /GL /Oi /arch:AVX2 /Zi /Gm- /Zc:inline /Qpar")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" "${MSVC_COMPILER_FLAGS}")
	add_link_options("/LTCG")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	add_compile_options(-fopenmp -mavx2 -mveclibabi=svml -mf16c -fpermissive -Wall)
	add_link_options(-lstdc++fs -fopenmp)
else()
	message(FATAL_ERROR "Your compiler is not supported yet, CMake will exit.")
endif()

# OpenGL
include_directories(dependencies/glfw/include)

find_package(GLEW REQUIRED)

# OpenColorIO
# There is an issue with OCIO V2 and vcpkg, so we will keep on using the shipped library instead
# find_package(OpenColorIO REQUIRED)
include_directories(dependencies/ociov2/include)

# OpenImageIO
find_package(OpenImageIO REQUIRED)

# FFMPEG
find_package(FFMPEG REQUIRED)

# Hack to get the vcpkg include directory working
find_path(VCPKG_INCLUDE_DIR zlib.h)
include_directories(${VCPKG_INCLUDE_DIR})

message(STATUS "Vcpkg include directory : ${VCPKG_INCLUDE_DIR}")

# OpenEXR
find_package(IlmBase REQUIRED)
find_package(OpenEXR REQUIRED)

# imf headers have some <> include that requires this OpenEXR include to be found
if(WIN32)
	include_directories(${OPENIMAGEIO_INCLUDES}/OpenEXR)
elseif(UNIX)
	include_directories(${VCPKG_INCLUDE_DIR}/OpenEXR)
endif()

include_directories(src)
include_directories(imgui)

add_subdirectory(dependencies)
add_subdirectory(imgui)
add_subdirectory(src)