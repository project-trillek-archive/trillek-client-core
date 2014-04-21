CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(trillek-client-core)

# Set the directory of cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

# Put the executable in the bin folder
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# find all source files in the src directory
FILE(GLOB_RECURSE TCC_SRC "src/*.cpp")

# Put the main_XXX.cpp files into a filter
FILE(GLOB TCC_MAIN_CPP "main/*.cpp")

# find all the files in the includes directory
FILE(GLOB_RECURSE TCC_INCLUDES "include/*.h" "include/*.hpp")

#Check if we can build with the current chaintool
INCLUDE(SetCompilerFlags)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/include/)
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/")

# If we are on windows add in the local search directories as well.
IF (WIN32 AND NOT MINGW) # Windows
  SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/lib/include/)
  INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib/include/")
  IF (CMAKE_CL_64)
    LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib/x64/debug" "${CMAKE_SOURCE_DIR}/lib/x64/release")
    SET(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/lib/x64/debug ${CMAKE_SOURCE_DIR}/lib/x64/release)
  ELSE (CMAKE_CL_64)
    LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib/x86/debug" "${CMAKE_SOURCE_DIR}/lib/x86/release")
    SET(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/lib/x86/debug ${CMAKE_SOURCE_DIR}/lib/x86/release)
  ENDIF (CMAKE_CL_64)
ENDIF (WIN32 AND NOT MINGW)

SET(TCC_BUILD_TESTS CACHE BOOL "Parse the tests directory")

IF (TCC_BUILD_TESTS)
  ADD_SUBDIRECTORY(tests)
ENDIF (TCC_BUILD_TESTS)

# define all required external libraries
set(TCC_ALL_LIBS
  )

FOREACH(MAIN_CPP ${TCC_MAIN_CPP})
    GET_FILENAME_COMPONENT(TEST_NAME ${MAIN_CPP} NAME_WE)
    if(TEST_NAME STREQUAL "Client_main")
        SET(TEST_NAME "Trillek_Client")
        SET(BUILDTARGET "CLIENT")
    endif(TEST_NAME STREQUAL "Client_main")

    if(TEST_NAME STREQUAL "Server_main")
        SET(TEST_NAME "Trillek_Server")
        SET(BUILDTARGET "SERVER")
    endif(TEST_NAME STREQUAL "Server_main")

    if(TEST_NAME STREQUAL "main")
        SET(TEST_NAME "Trillek")
        SET(BUILDTARGET "STANDALONE")
    endif(TEST_NAME STREQUAL "main")

    MESSAGE(STATUS "Processing: ${TEST_NAME}")

    # if just building an exe use all the source and libraries
    ADD_EXECUTABLE(${TEST_NAME}
      ${MAIN_CPP}
      ${TCC_SRC}
      ${TCC_INCLUDES}
      )

    # Set a preprocessor variable
    set_property(
        TARGET ${TEST_NAME}
        PROPERTY COMPILE_DEFINITIONS ${BUILDTARGET}
    )

    # Link the executable to all required libraries
    TARGET_LINK_LIBRARIES(${TEST_NAME} ${TCC_ALL_LIBS})
ENDFOREACH(MAIN_CPP)