#ifndef LUA_GLM_H_PP
#define LUA_GLM_H_PP

#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "systems/physics.hpp"

template<>
struct luaU_Impl<trillek::physics::Force> {
    static trillek::physics::Force luaU_check(lua_State* L, int index) {
        trillek::physics::Force f = {
            luaU_getfield<double>(L, index, "x"),
            luaU_getfield<double>(L, index, "y"),
            luaU_getfield<double>(L, index, "z") };
        return f;
    }

    static trillek::physics::Force luaU_to(lua_State* L, int index) {
        trillek::physics::Force f = {
            luaU_getfield<double>(L, index, "x"),
            luaU_getfield<double>(L, index, "y"),
            luaU_getfield<double>(L, index, "z") };
        return f;
    }

    static void luaU_push(lua_State* L, const trillek::physics::Force& val) {
        lua_newtable(L);
        luaU_setfield<double>(L, -1, "x", val.x);
        luaU_setfield<double>(L, -1, "y", val.y);
        luaU_setfield<double>(L, -1, "z", val.z);
    }
};

#endif
