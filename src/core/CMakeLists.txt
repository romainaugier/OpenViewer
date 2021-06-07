# CORE #

file(GLOB_RECURSE src_files *.cpp)
file(GLOB_RECURSE h_files *.h)

add_library(core ${src_files} ${h_files})

target_link_libraries(core OpenImageIO::OpenImageIO)
target_link_libraries(core wuffs)