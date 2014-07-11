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

FIND_PACKAGE(GLM REQUIRED)
# Force using radians as degrees are depcrecated
# Bullet is builet using doubles
ADD_DEFINITIONS(-DGLM_FORCE_RADIANS -DBT_USE_DOUBLE_PRECISION)

FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLFW3 REQUIRED)
FIND_PACKAGE(RapidJSON REQUIRED)
FIND_PACKAGE(Bullet REQUIRED)
FIND_PACKAGE(ALURE REQUIRED)
FIND_PACKAGE(OpenAL REQUIRED)

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

SET(TCC_BUILD_TESTS CACHE BOOL "Parse the tests directory")

IF (TCC_BUILD_TESTS)
  ADD_SUBDIRECTORY(tests)
ENDIF (TCC_BUILD_TESTS)

# define all required external libraries
set(TCC_ALL_LIBS
	${OPENGL_LIBRARIES}
	${GLFW3_LIBRARIES}
	${X11_LIBRARIES}
	${OSX_LIBRARIES}
	${GLEW_LIBRARIES}
	${BULLET_LINEARMATH_LIBRARIES}
	${BULLET_COLLISION_LIBRARIES}
	${BULLET_DYNAMICS_LIBRARIES}
	${ALURE_LIBRARIES}
	${OPENAL_LIBRARIES}
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

    if(TEST_NAME STREQUAL "Standalone_main")
        SET(TEST_NAME "Trillek_Standalone")
        SET(BUILDTARGET "STANDALONE")
    endif(TEST_NAME STREQUAL "Standalone_main")

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