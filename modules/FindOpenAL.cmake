# FindOpenAL.cmake
#
# Copyright (c) 2013, Meisaka Yukara

include(FindPackageHandleStandardArgs)

find_path(OPENAL_INCLUDE_DIR 
	NAMES al.h alc.h
	PATH_SUFFIXES AL
	)
if (NOT OPENAL_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find the OpenAL headers" )
endif (NOT OPENAL_INCLUDE_DIR)

FIND_LIBRARY(OPENAL_LIBRARIES 
	NAMES OpenAL32 openal libopenal libopenal.so
    PATH_SUFFIXES x86 lib64 x64
	)
if (NOT OPENAL_LIBRARIES)
    message(FATAL_ERROR "Could not find the OpenAL library" )
endif (NOT OPENAL_LIBRARIES)


find_package_handle_standard_args(OPENAL DEFAULT_MSG OPENAL_INCLUDE_DIR OPENAL_LIBRARIES)
mark_as_advanced(OPENAL_INCLUDE_DIR OPENAL_LIBRARIES)
