# FindAlure.cmake
#
# Copyright (c) 2013, Meisaka Yukara, Luis Panadero Guardeño

include(FindPackageHandleStandardArgs)

find_path(ALURE_INCLUDE_DIR
    NAMES alure.h
    PATHS /usr/include /usr/local/include /sw/include /opt/local/include
    PATH_SUFFIXES AL OpenAL
    )

if (NOT ALURE_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find the Alure headers" )
endif (NOT ALURE_INCLUDE_DIR)

FIND_LIBRARY(ALURE_LIBRARIES
    NAMES ALURE ALURE32 Alure alure libalure libalure.so
    PATH_SUFFIXES x86 lib64 x64
    )

if (NOT ALURE_LIBRARIES)
    message(FATAL_ERROR "Could not find the Alure library" )
endif (NOT ALURE_LIBRARIES)

find_package_handle_standard_args(ALURE DEFAULT_MSG ALURE_INCLUDE_DIR ALURE_LIBRARIES)
mark_as_advanced(ALURE_INCLUDE_DIR ALURE_LIBRARIES)
