CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(trillek-client-core)

# Put the executable in the bin folder
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Set the directory of Find<Library>.cmake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

# find all source files in the src directory
FILE(GLOB_RECURSE TCC_SRC "src/*.cpp")

# find all the files in the includes directory
FILE(GLOB_RECURSE TCC_INCLUDES "include/*.h" "include/*.hpp")

# check for gcc version to set c++11 or c++0x.
# thanks to http://stackoverflow.com/questions/10984442/how-to-detect-c11-support-of-a-compiler-with-cmake .
IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	IF (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
		MESSAGE("Supported GCC!")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	ENDIF (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7)
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")	# Clang 3.3 and up support c++11.  
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

# if just building an exe use all the source and libraries
ADD_EXECUTABLE("Trillek_Client"
	${TCC_SRC}
	${TCC_INCLUDES}
	)
# Link the executable to all required libraries
TARGET_LINK_LIBRARIES("Trillek_Client" ${TCC_ALL_LIBS})
