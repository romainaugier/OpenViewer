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


# Linux std lib
if(UNIX)
set(CMAKE_CXX_STANDARD_LIBRARIES "-ldl")
endif()

# Project
project(OpenViewer)

# cpp
set(CMAKE_CXX_STANDARD 17)

# to correct the bug appearing in half.h
if(WIN32)
add_definitions(-DOPENEXR_DLL)
endif()

# OpenViewer version str
if(DEFINED OPENVIEWER_VERSION_STR)
	add_definitions("-DOV_VERSION_STR=\"${OPENVIEWER_VERSION_STR}\"")
	message(STATUS "Building OpenViewer version : ${OPENVIEWER_VERSION_STR}")
	else()
	add_definitions("-DOV_VERSION_STR=\"Debug\"")
	message(STATUS "Building OpenViewer version : Debug")
endif()
	
# Compilation flags
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -DOPENEXR_DLL)
	set(MSVC_COMPILER_FLAGS "/fp:fast /Os /Oy /GT /GL /Oi /Zi /Gm- /Zc:inline /Qpar")

	if(DEFINED AVX2)
		add_definitions("-DOV_VECTOR_EXT=\"avx2\"")
		set(MSVC_COMPILER_FLAGS "${MSVC_COMPILER_FLAGS} /arch:AVX2")
	elseif(DEFINED AVX)
		add_definitions("-DOV_VECTOR_EXT=\"avx\"")
		set(MSVC_COMPILER_FLAGS "${MSVC_COMPILER_FLAGS} /arch:AVX")
	elseif(DEFINED SSE2)
		add_definitions("-DOV_VECTOR_EXT=\"sse2\"")
		set(MSVC_COMPILER_FLAGS "${MSVC_COMPILER_FLAGS} /arch:SSE2")
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" "${MSVC_COMPILER_FLAGS} /O2")
	add_link_options("/LTCG")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	add_compile_options(-fopenmp -mveclibabi=svml -mf16c -fpermissive -Wall)
	add_link_options(-lstdc++fs -fopenmp)
else()
	message(FATAL_ERROR "Your compiler is not supported yet, CMake will exit.")
endif()

# OpenGL
include_directories(dependencies/glfw/include)

# Boost stacktrace
set(_Boost_STACKTRACE_BASIC_HEADERS     "boost/stacktrace.hpp")
set(_Boost_STACKTRACE_BACKTRACE_HEADERS "boost/stacktrace.hpp")
set(_Boost_STACKTRACE_ADDR2LINE_HEADERS "boost/stacktrace.hpp")
set(_Boost_STACKTRACE_NOOP_HEADERS      "boost/stacktrace.hpp")

find_package(Boost COMPONENTS stacktrace_basic stacktrace_backtrace stacktrace_addr2line stacktrace_noop)


# Glew
find_package(GLEW CONFIG REQUIRED)

# GLFW
find_package(glfw3 CONFIG REQUIRED)

# Json
find_package(nlohmann_json CONFIG REQUIRED)

# OpenColorIO
# There is an issue with OCIO V2 and vcpkg, so we will keep on using the shipped library instead
#find_package(OpenColorIO REQUIRED)
include_directories(dependencies/ociov2/include)

# OpenImageIO
find_package(OpenImageIO CONFIG REQUIRED)

# FFMPEG
find_package(FFMPEG REQUIRED)

# Hack to get the vcpkg include directory working
find_path(VCPKG_INCLUDE_DIR zlib.h)
include_directories(${VCPKG_INCLUDE_DIR})

message(STATUS "Vcpkg include directory : ${VCPKG_INCLUDE_DIR}")

# OpenEXR
find_package(IlmBase CONFIG REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)

# imf headers have some <> include that requires this OpenEXR include to be found
include_directories(${VCPKG_INCLUDE_DIR}/OpenEXR)

include_directories(src)
include_directories(imgui)

add_subdirectory(dependencies)
add_subdirectory(imgui)
add_subdirectory(src)