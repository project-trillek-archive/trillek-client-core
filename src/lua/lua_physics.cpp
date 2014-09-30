#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "lua/lua_force.hpp"

#include "systems/physics.hpp"
#include "trillek-game.hpp"

namespace trillek {
namespace script {

int Physics_get(lua_State* L) {
    luaW_push<physics::PhysicsSystem>(L, &TrillekGame::GetPhysicsSystem());
    return 1;
}

int SetForce(lua_State* L) {
    auto physSys = luaW_check<physics::PhysicsSystem>(L, 1);
    int entity_id = luaL_checkint(L, 2);
    physics::Force f = luaU_check<physics::Force>(L, 3);
    physSys->SetForce(entity_id, f);

    return 0;
}

int SetTorque(lua_State* L) {
    auto physSys = luaW_check<physics::PhysicsSystem>(L, 1);
    int entity_id = luaL_checkint(L, 2);
    physics::Force t = luaU_check<physics::Force>(L, 3);
    physSys->SetTorque(entity_id, t);

    return 0;
}

int RemoveForce(lua_State* L) {
    auto physSys = luaW_check<physics::PhysicsSystem>(L, 1);
    int entity_id = luaL_checkint(L, 2);
    physSys->RemoveForce(entity_id);

    return 0;
}

int RemoveTorque(lua_State* L) {
    auto physSys = luaW_check<physics::PhysicsSystem>(L, 1);
    int entity_id = luaL_checkint(L, 2);
    physSys->RemoveTorque(entity_id);

    return 0;
}

int SetGravity(lua_State* L) {
    auto physSys = luaW_check<physics::PhysicsSystem>(L, 1);
    int entity_id = luaL_checkint(L, 2);
    if (lua_type(L, 3) == LUA_TNIL) {
        physSys->SetGravity(entity_id, nullptr);
    }
    else {
        physics::Force f = luaU_check<physics::Force>(L, 3);
        physSys->SetGravity(entity_id, &f);
    }

    return 0;
}



static luaL_Reg Physics_table[] =
{
    { "Get", Physics_get },
    { nullptr, nullptr } // table end marker
};

static luaL_Reg Physics_metatable[] =
{
    { "set_force", SetForce },
    { "set_torque", SetTorque },
    { "remove_force", RemoveForce },
    { "remove_torque", RemoveTorque },
    { "set_gravity", SetGravity },
    { nullptr, nullptr } // table end marker
};

int luaopen_PhysSys(lua_State* L) {
    luaW_register<physics::PhysicsSystem>(L,
        "Physics",
        Physics_table,
        Physics_metatable,
        nullptr // If your class has a default constructor you can omit this argument,
        // LuaWrapper will generate a default allocator for you.
        );
    return 1;
}

} // End of script
} // End of trillek
