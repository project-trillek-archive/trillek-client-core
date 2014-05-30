# - Find the OpenGL Extension Wrangler Library (GLEW)
# This module defines the following variables:
#  GLEW_INCLUDE_DIRS - include directories for GLEW
#  GLEW_LIBRARIES - libraries to link against GLEW
#  GLEW_FOUND - true if GLEW has been found and can be used

#=============================================================================
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(GLEW_INCLUDE_DIR GL/glew.h)
if(USE_STATIC_GLEW)
set(GLEW_DEBUG_NAMES glew32sd)
set(GLEW_NAMES glew32s)
add_definitions(-DGLEW_STATIC)
else(USE_STATIC_GLEW)
set(GLEW_DEBUG_NAMES glew32d)
set(GLEW_NAMES GLEW glew32 glew)
endif(USE_STATIC_GLEW)

# Reset the variables so that find_library will rescan in case we changed from static to none (or vice versa).
UNSET(GLEW_LIBRARY)
UNSET(GLEW_DEBUG_LIBRARY)

find_library(GLEW_LIBRARY NAMES ${GLEW_NAMES} PATH_SUFFIXES lib64)
find_library(GLEW_DEBUG_LIBRARY NAMES ${GLEW_DEBUG_NAMES} PATH_SUFFIXES lib64)

IF(GLEW_DEBUG_LIBRARY STREQUAL GLEW_DEBUG_LIBRARY-NOTFOUND)
SET(GLEW_DEBUG_LIBRARY GLEW_LIBRARY)
ENDIF(GLEW_DEBUG_LIBRARY STREQUAL GLEW_DEBUG_LIBRARY-NOTFOUND)

set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
set(GLEW_LIBRARIES debug ${GLEW_DEBUG_LIBRARY} optimized ${GLEW_LIBRARY})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW DEFAULT_MSG GLEW_LIBRARIES GLEW_INCLUDE_DIR)

mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY GLEW_DEBUG_LIBRARY)
