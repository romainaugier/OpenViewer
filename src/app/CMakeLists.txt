# APP #

file(GLOB_RECURSE app_srcs *.cpp)
file(GLOB_RECURSE app_h *.h)

add_library(app ${app_srcs} ${app_h})

target_link_libraries(app imgui)