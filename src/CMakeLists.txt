# SRC #

add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(app)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} app core)

# OpenColorIO DLL
if(WIN32)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		message(FATAL_ERROR "OCIO : Target platform Windows x86 not supported yet")
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/dependencies/ociov2/dll-windows-x64/OpenColorIO_2_1.dll
                ${CMAKE_CURRENT_BINARY_DIR}/OpenColorIO_2_1.dll)
	endif()
elseif(UNIX)
	message(FATAL_ERROR "OCIO : Target platform Linux not supported yet")
endif()