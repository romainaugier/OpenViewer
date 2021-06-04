# IMGUI #

file(GLOB_RECURSE imgui_src *.c *.cpp)
file(GLOB_RECURSE imgui_h *.h)

add_library(imgui ${imgui_h} ${imgui_src})

# link against glfw library for both x86 and x64, and linux
if(WIN32)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		target_link_libraries(imgui ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-vc2019-x86/glfw3.lib)
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		target_link_libraries(imgui ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-vc2019-x64/glfw3.lib)
	endif()
elseif(UNIX)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		message(FATAL_ERROR "GLFW : Target platform Linux x86 not supported yet")
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		target_link_libraries(imgui ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-linux/libglfw.so)
	endif()
endif()
