cmake_minimum_required(VERSION 2.6)

project(TrillekClientCore)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

add_subdirectory(libs/glfw3)
include_directories(libs/glfw3/include)

make_directory("bin")
add_executable(bin/TrillekClientCore main.cxx)

target_link_libraries(bin/TrillekClientCore glfw ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES})
