# CORE #

file(GLOB_RECURSE src_files *.cpp)
file(GLOB_RECURSE h_files *.h)

add_library(core ${src_files} ${h_files})

target_link_libraries(core OpenImageIO::OpenImageIO OpenImageIO::OpenImageIO_Util)
target_link_libraries(core wuffs utils)
target_link_libraries(core IlmBase::Iex IlmBase::Half IlmBase::Imath IlmBase::IexMath)
target_link_libraries(core OpenEXR::IlmImf OpenEXR::IlmImfUtil OpenEXR::IlmImfConfig)
target_link_libraries(core ${FFMPEG_LIBRARIES})
# target_link_libraries(core OpenColorIO::OpenColorIO)
target_link_libraries(core ocio_gpu_helper)

if(WIN32)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		message(FATAL_ERROR "OCIO : Target platform Windows x86 not supported yet")
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		target_link_libraries(core ocio_gpu_helper)
		target_link_libraries(core ${CMAKE_SOURCE_DIR}/dependencies/ociov2/lib-windows-x64/OpenColorIO_2_1.lib)
	endif()
elseif(UNIX)
	target_link_libraries(core ${CMAKE_SOURCE_DIR}/dependencies/ociov2/lib-linux-x64/libOpenColorIO2_1.so)
	target_link_libraries(core ocio_gpu_helper)
endif()