# IMGUI #

set(imgui_files
	gl3w.c
	imconfig.h
	imgui.h
	imgui.cpp
	imgui_demo.cpp
	imgui_impl_glfw.h
	imgui_impl_glfw.cpp
	imgui_impl_opengl3.h
	imgui_impl_opengl3.cpp
	imgui_internal.h
	imgui_tables.cpp
	imgui_widgets.cpp
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
)

add_library(imgui ${imgui_files})

# link against glfw library for both x86 and x64, and linux
if(WIN32)
	if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
		target_link_libraries(imgui ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-vc2019-x86/glfw3.lib)
	elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		target_link_libraries(imgui ${CMAKE_SOURCE_DIR}/dependencies/glfw/lib-vc2019-x64/glfw3.lib)
	endif()
elseif(UNIX)
	message("Linux")
endif()
