# SRC #

add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(app)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} app core)

# DLL's copy to bin directory
if(WIN32)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		message(FATAL_ERROR "OCIO : Target platform Windows x86 not supported yet")
        set(DEPENDENCIES_DLLS
        ${CMAKE_SOURCE_DIR}/dependencies/ociov2/dll-windows-x86/OpenColorIO_2_1.dll
        ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-vc2019-x86/glew32.dll
        ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-vc2019-x86/freeglut.dll
        )

        foreach(file_i ${DEPENDENCIES_DLLS})
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy 
                ${file_i} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
        endforeach( file_i )
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(DEPENDENCIES_DLLS
        ${CMAKE_SOURCE_DIR}/dependencies/ociov2/dll-windows-x64/OpenColorIO_2_1.dll
        ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-vc2019-x64/glew32.dll
        ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-vc2019-x64/freeglut.dll
        )

        foreach(file_i ${DEPENDENCIES_DLLS})
            add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy 
                ${file_i} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
        endforeach( file_i )
	endif()
elseif(UNIX)
	message(FATAL_ERROR "OCIO : Target platform Linux not supported yet")

endif()

# ImGui initialization file
add_custom_command(
TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/imgui.ini
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)