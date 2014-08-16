#include "systems/lua-system.hpp"
#include <iostream>

namespace trillek {
namespace script {

int luaopen_Transform(lua_State*);
int luaopen_LuaSys(lua_State*);

void LuaSystem::RegisterTypes() {
    // TODO: Move this into a location that makes more sense.
    RegisterSystem(&luaopen_Transform);
    RegisterSystem(&luaopen_LuaSys);
}

} // End of script
} // End of trillek
