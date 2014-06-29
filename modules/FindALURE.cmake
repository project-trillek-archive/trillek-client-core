# - Find ALURE
# Find ALURE includes and library
#
#  ALURE_INCLUDE_DIR - where to find alure.h
#  ALURE_LIBRARIES   - List of libraries when using dumb
#  ALURE_FOUND       - True if include and lib was found

FIND_PATH(ALURE_INCLUDE_DIR AL/alure.h
		  PATHS "${ALURE_DIR}"
		  PATH_SUFFIXES include
		  )
		  
FIND_LIBRARY(ALURE_LIBRARIES NAMES ALURE ALURE32
		  PATHS "${ALURE_DIR}"
		  PATH_SUFFIXES lib
		  )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ALURE DEFAULT_MSG ALURE_LIBRARIES ALURE_INCLUDE_DIR)		
