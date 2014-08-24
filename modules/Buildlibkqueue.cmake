PROJECT(libkqueue)
include(ExternalProject)

set(LIBKQUEUE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/libkqueue")
set(LIBKQUEUE_BINARY_DIR "${EXECUTABLE_OUTPUT_PATH}")
set(LIBKQUEUE_LIBRARIES libkqueue.so)
IF (NOT WIN32 OR MINGW) # not windows
set(LIBKQUEUE_CONFIGURE_COMMAND autoreconf -i && ./configure --prefix=${LIBKQUEUE_BINARY_DIR})
ENDIF (NOT WIN32 OR MINGW)

ExternalProject_Add(libkqueue_project
  PREFIX ${LIBKQUEUE_PREFIX}
  GIT_REPOSITORY https://github.com/catageek/libkqueue.git
  GIT_TAG remove_include_config_h
  UPDATE_COMMAND ""
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ${LIBKQUEUE_CONFIGURE_COMMAND}
  INSTALL_DIR ${LIBKQUEUE_BINARY_DIR}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${LIBKQUEUE_BINARY_DIR}
)

add_library(libkqueue SHARED IMPORTED)
set_property(TARGET libkqueue PROPERTY IMPORTED_LOCATION ${LIBKQUEUE_BINARY_DIR})
add_dependencies(libkqueue libkqueue_project)

# Set the include directory
INCLUDE_DIRECTORIES(${LIBKQUEUE_BINARY_DIR}/include/kqueue)
