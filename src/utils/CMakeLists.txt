# UTILS #

file(GLOB_RECURSE src_files *.cpp)
file(GLOB_RECURSE h_files *.h)

add_library(utils ${src_files} ${h_files})