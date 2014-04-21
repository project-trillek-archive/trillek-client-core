project (TCCTests)

cmake_minimum_required(VERSION 2.8)

# Set the directory of cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

#Check if we can build with the current chaintool
INCLUDE(SetCompilerFlags)

IF(MINGW OR UNIX OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # gtest requires pthread on *nix
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF(MINGW OR UNIX OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/modules")

IF (NOT (DEFINED TCC_BUILD_TESTS))
  MESSAGE("Called independently.")
  MESSAGE("The include directory ${CMAKE_SOURCE_DIR}/../include may not be correct.")
  SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/../include)
  INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../include")
ELSE (NOT (DEFINED TCC_BUILD_TESTS))
  MESSAGE("Called from root.")
ENDIF (NOT (DEFINED TCC_BUILD_TESTS))

# If we are on windows add in the local search directories as well.
IF (WIN32 AND NOT MINGW) # Windows
  SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/lib/include/)
  INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/lib/include/")
  IF (CMAKE_CL_64)
    LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib")
    SET(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
  ELSE (CMAKE_CL_64)
    LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib")
    SET(CMAKE_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
  ENDIF (CMAKE_CL_64)
ENDIF (WIN32 AND NOT MINGW)

# Try to get GTest using a Env. variable, if not, with find_package
IF (DEFINED ENV{GTEST_ROOT})
  MESSAGE ("... using gtest found in $ENV{GTEST_ROOT}")
  # Example :
  # GTEST_ROOT=path/gtest-1.6.0 ;
  # export GTEST_ROOT
  ADD_SUBDIRECTORY ($ENV{GTEST_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/gtest)
  INCLUDE_DIRECTORIES ($ENV{GTEST_ROOT}/include $ENV{GTEST_ROOT})
  SET (GTEST_LIBRARIES gtest)

ELSE (DEFINED ENV{GTEST_ROOT})
  FIND_PACKAGE(GTEST REQUIRED)

ENDIF (DEFINED ENV{GTEST_ROOT})

file(GLOB TCCTests_SRC "tests/*.h" "main.cpp")

foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
   if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
   endif(${flag_var} MATCHES "/MD")
endforeach(flag_var)

ADD_EXECUTABLE(TCCTests ${TCCTests_SRC})

TARGET_LINK_LIBRARIES (TCCTests ${GTEST_LIBRARIES})

MESSAGE("Tests' Cmake configured.")
