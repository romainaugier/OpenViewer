# SRC #

add_subdirectory(core)
add_subdirectory(app)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} app core)