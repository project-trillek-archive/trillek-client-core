#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "trillek-game.hpp"

namespace trillek {
namespace script {

int OS_get(lua_State* L) {
    luaW_push<OS>(L, &TrillekGame::GetOS());
    return 1;
}

int ToggleMouseLock(lua_State* L) {
    auto os = luaW_check<OS>(L, 1);
    os->ToggleMouseLock();

    return 0;
}

int CallWindowShouldClose(lua_State* L) {
    auto os = luaW_check<OS>(L, 1);
    os->SetWindowShouldClose();

    return 0;
}

static luaL_Reg OS_table[] =
{
    { "Get", OS_get },
    { nullptr, nullptr } // table end marker
};

static luaL_Reg OS_metatable[] =
{
    { "ToggleMouseLock", ToggleMouseLock },
    { "Terminate", CallWindowShouldClose },
    { nullptr, nullptr } // table end marker
};

int luaopen_OSSys(lua_State* L) {
    luaW_register<OS>(L,
        "OS",
        OS_table,
        OS_metatable,
        nullptr // If your class has a default constructor you can omit this argument,
        // LuaWrapper will generate a default allocator for you.
        );
    return 1;
}

} // End of script
} // End of trillek
