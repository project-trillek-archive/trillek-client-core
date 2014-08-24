PROJECT(netport)
include(ExternalProject)

set(NETPORT_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/netport")
set(NETPORT_BINARY_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
set(NETPORT_LIBRARIES libnetport.so)

ExternalProject_Add(netport_project
  PREFIX ${NETPORT_PREFIX}
  GIT_REPOSITORY https://github.com/Meisaka/NetPort.git
  GIT_TAG develop
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CMAKE_CACHE_ARGS -DCMAKE_CXX_FLAGS:STRING=-std=c++11 -DBUILD_SHARED_LIBS:BOOL=TRUE
INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory lib ${NETPORT_BINARY_DIR} COMMAND ${CMAKE_COMMAND} -E copy_directory include ${EXECUTABLE_OUTPUT_PATH}/include
)

add_library(netport SHARED IMPORTED)
set_property(TARGET netport PROPERTY IMPORTED_LOCATION ${NETPORT_BINARY_DIR})
add_dependencies(netport netport_project)

# Set the include directory
INCLUDE_DIRECTORIES(${EXECUTABLE_OUTPUT_PATH}/include)
