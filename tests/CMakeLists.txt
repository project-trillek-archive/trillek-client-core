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
"${TCCSRC}/graphics/render-layer.cpp"
"${TCCSRC}/graphics/render-list.cpp"
"${TCCSRC}/graphics/light.cpp"
"${TCCSRC}/graphics/material.cpp"
"${TCCSRC}/graphics/texture.cpp"
"${TCCSRC}/graphics/animation.cpp"
"${TCCSRC}/physics/collidable.cpp"
"${TCCSRC}/transform.cpp"
"${TCCSRC}/trillek-game.cpp"
"${TCCSRC}/os.cpp"
"${TCCSRC}/opengl.cpp"
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
