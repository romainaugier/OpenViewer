# SRC #

add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(app)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} app core)

# cpp 17
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# DLL's/SO's copy to bin directory
if(WIN32)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		message(FATAL_ERROR "OCIO : Target platform Windows x86 not supported yet")
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
                set(DEPENDENCIES_DLLS ${CMAKE_SOURCE_DIR}/dependencies/ociov2/dll-windows-x64/OpenColorIO_2_1.dll)

                foreach(file_i ${DEPENDENCIES_DLLS})
                add_custom_command(
                        TARGET ${PROJECT_NAME}
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy 
                        ${file_i} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
                endforeach( file_i )
	endif()
elseif(UNIX)
        if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
                message(FATAL_ERROR "Target platform Linux x86 not supported yet")
        elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
                set(DEPENDENCIES_SO
                ${CMAKE_SOURCE_DIR}/dependencies/ociov2/lib-linux-x64/libOpenColorIO2_1.so)

                foreach(file_i ${DEPENDENCIES_SO})
                add_custom_command(
                        TARGET ${PROJECT_NAME}
                        POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy 
                        ${file_i} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
                endforeach( file_i )
        endif()
endif()

# ImGui initialization file
# add_custom_command(
#         TARGET ${PROJECT_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy
#         ${CMAKE_SOURCE_DIR}/ui/imgui.ini
#         $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# OCIO default configuration
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/configs
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/configs)

# GLSL Shaders
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)