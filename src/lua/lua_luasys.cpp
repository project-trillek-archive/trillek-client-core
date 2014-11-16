#include "systems/lua-system.hpp"
#include "trillek-game.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

namespace trillek {
namespace script {

int LuaSys_Get(lua_State* L) {
    luaW_push<LuaSystem>(L, &TrillekGame::GetLuaSystem());
    return 1;
}

int LuaSys_LoadFile(lua_State* L) {
    LuaSystem* sys = luaW_check<LuaSystem>(L, 1);
    std::string fname = luaL_checkstring(L, 2);

    luaU_push<bool>(L, sys->LoadFile(fname));

    return 1;
}

int LuaSys_Subscribe(lua_State* L) {
    LuaSystem* sys = luaW_check<LuaSystem>(L, 1);
    int eventType = luaL_checkint(L, 2);
    std::string function = luaL_checkstring(L, 3);

    sys->RegisterEventSubscriber(eventType, function);

    return 0;
}

static luaL_Reg LuaSystable[] =
{
    { "Get", LuaSys_Get },
    { nullptr, nullptr } // table end marker
};

static luaL_Reg LuaSys_metatable[] =
{
    { "LoadScriptFile", LuaSys_LoadFile },
    { "Subscribe", LuaSys_Subscribe },
    { nullptr, nullptr } // table end marker
};

int luaopen_LuaSys(lua_State* L) {
    luaW_register<LuaSystem>(L,
        "LuaSys",
        LuaSystable,
        LuaSys_metatable,
        nullptr // If your class has a default constructor you can omit this argument,
        // LuaWrapper will generate a default allocator for you.
        );
    return 1;
}

} // End of script
} // End of trillek
