# CORE #

file(GLOB_RECURSE src_files *.cpp)
file(GLOB_RECURSE h_files *.h)

add_library(core ${src_files} ${h_files})

target_link_libraries(core OpenImageIO::OpenImageIO)
target_link_libraries(core wuffs utils)
target_link_libraries(core IlmBase::Iex IlmBase::Half IlmBase::Imath IlmBase::IexMath)
target_link_libraries(core OpenEXR::IlmImf OpenEXR::IlmImfUtil OpenEXR::IlmImfConfig)

if(WIN32)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		message(FATAL_ERROR "OCIO : Target platform Windows x86 not supported yet")
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		target_link_libraries(core ${CMAKE_SOURCE_DIR}/dependencies/ociov2/lib-windows-x64/OpenColorIO_2_1.lib)
		target_link_libraries(core ocio_gpu_helper)
	endif()
elseif(UNIX)
	message(FATAL_ERROR "OCIO : Target platform Linux not supported yet")
endif()