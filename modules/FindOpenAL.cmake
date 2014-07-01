# - Find OpenAL
# Find OpenAL includes and library
#
#  OpenAL_INCLUDE_DIR - where to find OpenAL.h
#  OpenAL_LIBRARIES   - List of libraries when using dumb
#  OpenAL_FOUND       - True if include and lib was found

FIND_PATH(OpenAL_INCLUDE_DIR AL/al.h
		  PATHS "${OpenAL_DIR}"
		  PATH_SUFFIXES include
		  )
		  
FIND_LIBRARY(OpenAL_LIBRARIES NAMES libOpenAL32.dll.a OpenAL32
		  PATHS "${OpenAL_DIR}"
		  PATH_SUFFIXES lib
		  )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenAL DEFAULT_MSG OpenAL_LIBRARIES OpenAL_INCLUDE_DIR)		
