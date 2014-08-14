# locate Lua Wrapper
FIND_PATH(LUAWAPPER_INCLUDE_DIR luawrapper/luawrapper.hpp
	HINTS
	$ENV{LUA_DIR}
	PATH_SUFFIXES include
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)
INCLUDE_DIRECTORIES(${LUAWAPPER_INCLUDE_DIR})

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LuaWrapper
	REQUIRED_VARS LUAWAPPER_INCLUDE_DIR
)
MARK_AS_ADVANCED(LUAWAPPER_INCLUDE_DIR)
