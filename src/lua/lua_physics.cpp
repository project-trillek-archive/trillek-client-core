#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "lua/lua_velocity.hpp"

#include "components/shared-component.hpp"
#include "systems/physics.hpp"
#include "transform.hpp"
#include "trillek-game.hpp"

namespace trillek {
namespace script {

int Physics_get(lua_State* L) {
    luaW_push<physics::PhysicsSystem>(L, &TrillekGame::GetPhysicsSystem());
    return 1;
}

int SetVelocity(lua_State* L) {
    auto physSys = luaW_check<physics::PhysicsSystem>(L, 1);
    int entity_id = luaL_checkint(L, 2);
    physics::VelocityStruct f = luaU_check<physics::VelocityStruct>(L, 3);
    auto player_orientation = component::Get<component::Component::GraphicTransform>(entity_id).GetOrientation();
    auto camera_orientation = glm::toMat4(std::move(player_orientation));
    physics::VelocityStruct g(camera_orientation * f.linear, camera_orientation * f.angular);
    auto v_ptr = component::Create<component::Component::Velocity>(std::move(g));
    physSys->AddCommand(entity_id, std::move(v_ptr));

    return 0;
}

int SetGravity(lua_State* L) {
    auto physSys = luaW_check<physics::PhysicsSystem>(L, 1);
    int entity_id = luaL_checkint(L, 2);
    if (lua_type(L, 3) == LUA_TNIL) {
        physSys->SetNormalGravity(entity_id);
    }
    else {
        physics::VelocityStruct f = luaU_check<physics::VelocityStruct>(L, 3);
        physSys->SetGravity(entity_id, f.GetLinear());
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
    { "set_velocity", SetVelocity },
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
