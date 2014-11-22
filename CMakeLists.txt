CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(trillek-client-core)

# Set the directory of cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/common/modules")

# Put the executable in the bin folder
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Install the libraries using bin folder as root
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")

# Set the directory for the header files
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/common/include)

# set options early
SET(TCC_BUILD_TESTS CACHE BOOL "Parse the tests directory")
SET(TCC_TEST_NETWORK_DISABLE OFF CACHE BOOL "Disable the network unit test")

# find all source files in the src directory
FILE(GLOB_RECURSE TCC_SRC "src/*.cpp" "common/src/*.cpp")

# Put the main_XXX.cpp files into a filter
FILE(GLOB TCC_MAIN_CPP "main/*.cpp")

# find all the files in the includes directory
FILE(GLOB_RECURSE TCC_INCLUDES "include/*.h" "include/*.hpp" "common/include/*.h"
	"common/include/*.hpp")

#Check if we can build with the current chaintool
INCLUDE(SetCompilerFlags)

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/" "${CMAKE_SOURCE_DIR}/common/include/")

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

FIND_PACKAGE(GLM REQUIRED)
# Force using radians as degrees is deprecated
# Bullet is built using doubles
ADD_DEFINITIONS(-DGLM_FORCE_RADIANS -DBT_USE_DOUBLE_PRECISION)

FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLFW3 REQUIRED)
FIND_PACKAGE(RapidJSON REQUIRED)
FIND_PACKAGE(Bullet REQUIRED)
FIND_PACKAGE(ALURE REQUIRED)
FIND_PACKAGE(OpenAL REQUIRED)
FIND_PACKAGE(Lua REQUIRED)
FIND_PACKAGE(LuaWrapper REQUIRED)
INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})

# Give these some dummy values and if the platform is LINUX or OSX they will be set accordingly.
SET(X11_LIBRARIES "")
SET(OSX_LIBRARIES "")

IF (NOT APPLE) # X11 and GLEW are not needed on OSX.
    FIND_PACKAGE(X11)
    SET(USE_STATIC_GLEW CACHE BOOL "Build against GLEW static (default no)")
    FIND_PACKAGE(GLEW REQUIRED) # We find GLEW here as OSX doesn't need it.
ENDIF (NOT APPLE)

IF (APPLE) # Mac OSX
    SET(GLEW_LIBRARY "") # Set a dummy value for GLEW.

    SET(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS macosx)

    # Need the 10.7 SDK or later.
    EXECUTE_PROCESS(COMMAND xcodebuild -sdk macosx -version SDKVersion OUTPUT_VARIABLE OSX_SDK_VERSION)
    IF (NOT (OSX_SDK_VERSION VERSION_GREATER 10.7 OR OSX_SDK_VERSION VERSION_EQUAL 10.7))
        MESSAGE(FATAL_ERROR "The installed version of Xcode does not support the 10.7 SDK or later.  Please upgrade Xcode and try again.")
    ENDIF (NOT (OSX_SDK_VERSION VERSION_GREATER 10.7 OR OSX_SDK_VERSION VERSION_EQUAL 10.7))

    # Configure the project to use the correct SDK.
    IF (XCODE_VERSION)
        SET(CMAKE_OSX_SYSROOT macosx)
    ELSE (XCODE_VERSION)
        # Non-Xcode generators need the full path.
        EXECUTE_PROCESS(COMMAND xcodebuild -sdk macosx -version Path | head -n 1 OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
        STRING(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
    ENDIF (XCODE_VERSION)

    # Can deploy back to 10.7, the first OS X to support the GL Core.
    SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)

    # Need Core Foundation and libobjc.
    SET(OSX_LIBRARIES "-framework CoreFoundation /usr/lib/libobjc.dylib")
ENDIF (APPLE)

IF (TCC_BUILD_TESTS)
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

    FILE(GLOB_RECURSE TCCTests_SRC "common/tests/src/*.cpp")
    FILE(GLOB_RECURSE TCCTests_INCLUDE "common/tests/tests/*.h" "common/tests/tests/*.hpp")
    SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/common/tests/)
    INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/common/tests/")
ENDIF (TCC_BUILD_TESTS)

LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# define all required external libraries
SET(Backend_LIBS
    ${BULLET_DYNAMICS_LIBRARIES}
    ${BULLET_COLLISION_LIBRARIES}
    ${BULLET_LINEARMATH_LIBRARIES}
    ${LUA_LIBRARIES}
    )

SET(Graphics_LIBS
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLFW3_LIBRARIES}
    )

SET(Frontend_LIBS
    ${X11_LIBRARIES}
    ${OSX_LIBRARIES}
    ${Graphics_LIBS}
    ${ALURE_LIBRARIES}
    ${OPENAL_LIBRARIES}
    )

ADD_SUBDIRECTORY (${CMAKE_SOURCE_DIR}/common/trillek-vcomputer-module)

SET(Backend_LIBS
    ${Backend_LIBS}
    VCOMPUTER
    )

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/common/trillek-vcomputer-module/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/common/trillek-vcomputer-module/src")

SET(Trillek_Server_LIBS ${Backend_LIBS})
SET(Trillek_Client_LIBS ${Backend_LIBS} ${Frontend_LIBS})
SET(Trillek_Standalone_LIBS ${Trillek_Client_LIBS})

# Visio C++ needs GLFW for the clock
IF (MSVC) # Visual C++
    SET(Trillek_Server_LIBS ${Trillek_Server_LIBS} ${Graphics_LIBS})
ENDIF (MSVC)

FOREACH(MAIN_CPP ${TCC_MAIN_CPP})
    GET_FILENAME_COMPONENT(T_NAME ${MAIN_CPP} NAME_WE)
    IF(T_NAME STREQUAL "client-main")
        SET(T_NAME "Trillek_Client")
	SET(BUILDTARGET "BUILD_CLIENT")
    ENDIF(T_NAME STREQUAL "client-main")

    IF(BUILDTARGET)
        MESSAGE(STATUS "Processing: ${T_NAME}")

        # if just building an exe use all the source and libraries
        ADD_EXECUTABLE(${T_NAME}
            ${MAIN_CPP}
            ${TCC_SRC}
            ${TCC_INCLUDES}
            )

        # Set a preprocessor variable
        SET_PROPERTY(TARGET ${T_NAME} PROPERTY COMPILE_DEFINITIONS ${BUILDTARGET} APPEND)
        # this other form is only supported in new versions of cmake
        #target_compile_definitions(${T_NAME} PRIVATE ${BUILDTARGET})

	ADD_DEPENDENCIES(${T_NAME} VCOMPUTER)

        # Link the executable to all required libraries
        TARGET_LINK_LIBRARIES(${T_NAME} ${${T_NAME}_LIBS})
    ENDIF(BUILDTARGET)

    UNSET(BUILDTARGET)
ENDFOREACH(MAIN_CPP)

if(TCC_BUILD_TESTS)
    MESSAGE(STATUS "Processing: TCCTests")
    ADD_EXECUTABLE(TCCTests main/tests-main.cpp ${TCCTests_SRC} ${TCC_SRC} ${TCCTests_INCLUDE})
    # Set a preprocessor variable
    SET_PROPERTY(TARGET TCCTests PROPERTY COMPILE_DEFINITIONS _STANDALONE_ APPEND)
    #target_compile_definitions(TCCTests PRIVATE _STANDALONE_)
    ADD_DEPENDENCIES(TCCTests VCOMPUTER)
    IF(TCC_TEST_NETWORK_DISABLE)
        SET_PROPERTY(TARGET TCCTests PROPERTY COMPILE_DEFINITIONS NO_NETWORK APPEND)
        #target_compile_definitions(TCCTests PRIVATE NO_NETWORK)
    ENDIF(TCC_TEST_NETWORK_DISABLE)

    TARGET_LINK_LIBRARIES(TCCTests ${GTEST_LIBRARIES} ${Trillek_Standalone_LIBS})
endif(TCC_BUILD_TESTS)
