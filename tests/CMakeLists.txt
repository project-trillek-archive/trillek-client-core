project (TCCTests)

cmake_minimum_required(VERSION 2.8)

# check for gcc version to set c++11 or c++0x.
# thanks to http://stackoverflow.com/questions/10984442/how-to-detect-c11-support-of-a-compiler-with-cmake .
IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  IF (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
    MESSAGE("Supported GCC!")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  ENDIF (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") # Clang 3.3 and up support c++11.
  MESSAGE("Clang Version: " ${CMAKE_CXX_COMPILER_VERSION})
  # On OS X, Clang 3.3 would be Clang/LLVM 5.0.
  IF (APPLE)
    SET(CLANG_MIN_VERSION 5.0)
  ELSE (APPLE)
    SET(CLANG_MIN_VERSION 3.3)
  ENDIF (APPLE)
  IF (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER CLANG_MIN_VERSION OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL CLANG_MIN_VERSION))
    # clang 3.3 is the first release that completley implements the c++11 standard.
    # However, most of c++11 was implemented in previous versions.
    MESSAGE("clang ${CMAKE_CXX_COMPILER_VERSION} does not completely support c++11. This may cause some problems in the future. We recommend upgrading to clang-3.3 (Xcode 5.0) or greater.")
  ENDIF (NOT (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER CLANG_MIN_VERSION OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL CLANG_MIN_VERSION))
  # compiler is clang 3.3 or higher. Force c++11 and use libc++.
  IF (XCODE_VERSION)
    SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++11)
    SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++)
  ELSE (XCODE_VERSION)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
  ENDIF (XCODE_VERSION)
  UNSET(CLANG_MIN_VERSION)
ELSEIF (MSVC AND (MSVC_VERSION GREATER 1699))
  MESSAGE("Supported Visual Studio!")
ELSE ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  MESSAGE(FATAL_ERROR "Your C++ compiler does not support C++11.")
ENDIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

IF(MINGW OR UNIX OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # gtest requires pthread on *nix
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF(MINGW OR UNIX OR ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/modules")

IF (NOT (DEFINED TCC_BUILD_TESTS))
  MESSAGE("Called independently.")
  MESSAGE("The include directory ${CMAKE_SOURCE_DIR}/../include may not be correct.")
  SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/../include)
  INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../include")
  SET(TCCSRC "${CMAKE_SOURCE_DIR}/../src")
ELSE (NOT (DEFINED TCC_BUILD_TESTS))
  MESSAGE("Called from root.")
  SET(TCCSRC "${CMAKE_SOURCE_DIR}/src")
  MESSAGE(STATUS "Source path is ${TCCSRC}")
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

file(GLOB_RECURSE TCCTests_SRC "tests/*.h" "main.cpp" "src/*.cpp")

file(GLOB TCCExtra_SRC
"${TCCSRC}/systems/resource-system.cpp"
"${TCCSRC}/systems/sound-system.cpp"
"${TCCSRC}/systems/graphics.cpp"
"${TCCSRC}/systems/physics.cpp"
"${TCCSRC}/systems/registration.cpp"
"${TCCSRC}/systems/component-factory.cpp"
"${TCCSRC}/systems/transform-system.cpp"
"${TCCSRC}/systems/meta-engine-system.cpp"
"${TCCSRC}/util/json-parser.cpp"
"${TCCSRC}/util/util.cpp"
"${TCCSRC}/util/checksum.cpp"
"${TCCSRC}/util/compression.cpp"
"${TCCSRC}/loaders/png.cpp"
"${TCCSRC}/resources/pixel-buffer.cpp"
"${TCCSRC}/resources/md5mesh.cpp"
"${TCCSRC}/resources/md5anim.cpp"
"${TCCSRC}/resources/mesh.cpp"
"${TCCSRC}/graphics/renderable.cpp"
"${TCCSRC}/graphics/shader.cpp"
"${TCCSRC}/graphics/material.cpp"
"${TCCSRC}/graphics/texture.cpp"
"${TCCSRC}/graphics/animation.cpp"
"${TCCSRC}/physics/collidable.cpp"
"${TCCSRC}/transform.cpp"
"${TCCSRC}/trillek-game.cpp"
"${TCCSRC}/os.cpp"
)

# For certain files we need the OpenGL symbols.
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLFW3 REQUIRED)
FIND_PACKAGE(Bullet REQUIRED)
FIND_PACKAGE(ALURE REQUIRED)
FIND_PACKAGE(OpenAL REQUIRED)
SET(X11_LIBRARIES "")
IF (NOT APPLE) # X11 and GLEW are not needed on OSX.
	FIND_PACKAGE(X11)
	FIND_PACKAGE(GLEW REQUIRED) # We find GLEW here as OSX doesn't need it.
ELSE (NOT APPLE)
	SET(GLEW_LIBRARY "") # Set a dummy value for GLEW.
ENDIF (NOT APPLE)

foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
endforeach(flag_var)

ADD_EXECUTABLE(TCCTests ${TCCTests_SRC} ${TCCExtra_SRC})

TARGET_LINK_LIBRARIES (
    TCCTests ${GTEST_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLFW3_LIBRARIES}
    ${X11_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${BULLET_LINEARMATH_LIBRARIES}
    ${BULLET_COLLISION_LIBRARIES}
    ${BULLET_DYNAMICS_LIBRARIES}
    ${ALURE_LIBRARIES}
    ${OPENAL_LIBRARIES}
)

MESSAGE("Tests' Cmake configured.")
