#
# Try to find GLFW library and include path.
# Once done this will define
#
# GLFW3_FOUND
# GLFW3_INCLUDE_PATH
# GLFW3_LIBRARY
#

IF(WIN32)
	FIND_PATH( GLFW3_INCLUDE_PATH GLFW/glfw3.h
		$ENV{PROGRAMFILES}/GLFW/include
		${GLFW_ROOT_DIR}/include
		DOC "The directory where GLFW/glfw3.h resides")

	FIND_LIBRARY( GLFW3_DEBUG_LIBRARY
		NAMES glfw3d
		PATHS
		$ENV{PROGRAMFILES}/GLFW/lib
		${GLFW_ROOT_DIR}/lib
		DOC "The GLFW library")
	
	FIND_LIBRARY( GLFW3_LIBRARY
		NAMES glfw3
		PATHS
		$ENV{PROGRAMFILES}/GLFW/lib
		${GLFW_ROOT_DIR}/lib
		DOC "The GLFW library")
ELSE(WIN32)
	FIND_PATH( GLFW3_INCLUDE_PATH GLFW/glfw3.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		${GLFW_ROOT_DIR}/include
		DOC "The directory where GLFW/glfw3.h resides")
	IF(APPLE)
		# For the case where GLFW is installed in a non-standard lib path such as /opt or /sw
		INCLUDE_DIRECTORIES( "${GLFW3_INCLUDE_PATH}" )
	ENDIF(APPLE)
	
	FIND_LIBRARY( GLFW3_LIBRARY
		NAMES libglfw.so libglfw.so.3 libglfw.so.3.0 glfw.3 glfw3
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		${GLFW_ROOT_DIR}/lib
		DOC "The GLFW library")

	SET(GLFW3_DEBUG_LIBRARY ${GLFW3_LIBRARY})
ENDIF(WIN32)

set(GLFW3_INCLUDE_PATH ${GLFW3_INCLUDE_PATH})
set(GLFW3_LIBRARIES debug "${GLFW3_DEBUG_LIBRARY}" optimized "${GLFW3_LIBRARY}")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLFW3 DEFAULT_MESSAGE GLFW3_LIBRARIES GLFW3_INCLUDE_PATH)
