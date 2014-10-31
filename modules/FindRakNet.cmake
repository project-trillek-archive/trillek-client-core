# FIND RakNet
# SET 
# RAKNET_INCLUDE_DIRS '-I'
# RAKNET_LIBRARY_DIRS '-L'
# RAKNET_LIBRARIES the '-l'

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
	message("-- looking for RakNet using PKG")
	pkg_check_modules(RakNet RakNet)
endif()

if (NOT RakNet_FOUND)
	message("-- Manualy looking for RakNet")

	# Set Search Paths
	set(RAKNET_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/raknet/Source)
	set(RAKNET_LIBRARY_DIRS ${CMAKE_SOURCE_DIR}/raknet/Lib/DLL)
	
	find_path(RAKNET_INCLUDE_DIR RakPeer.h ${RAKNET_INCLUDE_DIRS})
	if (RAKNET_INCLUDE_DIR)
		message("--  RakNet includes found")
	endif()

	find_library(RAKNET_LIBRARIES NAMES RakNetDLL PATHS ${RAKNET_LIBRARY_DIRS})
	if (RAKNET_LIBRARIES)
		message("--  RakNet lib found")
	endif()

	if (RAKNET_INCLUDE_DIR AND RAKNET_LIBRARIES)
		message("--  RakNet resolved!")
		INCLUDE_DIRECTORIES(${RAKNET_INCLUDE_DIR})
		set(RAKNET_FOUND TRUE)
	endif()	
endif()

if (NOT RAKNET_FOUND)
	message(FATAL_ERROR "RakNet not found!")
endif()
