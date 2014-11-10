#ifndef LUA_GLM_H_PP
#define LUA_GLM_H_PP

#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "systems/physics.hpp"

template<>
struct luaU_Impl<trillek::physics::VelocityStruct> {
    static trillek::physics::VelocityStruct luaU_check(lua_State* L, int index) {
        trillek::physics::VelocityStruct f = {
            glm::vec4(luaU_getfield<double>(L, index, "x"),
            luaU_getfield<double>(L, index, "y"),
            luaU_getfield<double>(L, index, "z"),0),
            glm::vec4(luaU_getfield<double>(L, index, "rx"),
            luaU_getfield<double>(L, index, "ry"),
            luaU_getfield<double>(L, index, "rz"),0) };
        return f;
    }

    static trillek::physics::VelocityStruct luaU_to(lua_State* L, int index) {
        trillek::physics::VelocityStruct f = {
            glm::vec4(luaU_getfield<double>(L, index, "x"),
            luaU_getfield<double>(L, index, "y"),
            luaU_getfield<double>(L, index, "z"),0),
            glm::vec4(luaU_getfield<double>(L, index, "rx"),
            luaU_getfield<double>(L, index, "ry"),
            luaU_getfield<double>(L, index, "rz"),0) };
        return f;
    }

    static void luaU_push(lua_State* L, const trillek::physics::VelocityStruct& val) {
        lua_newtable(L);
        luaU_setfield<double>(L, -1, "x", val.linear.x);
        luaU_setfield<double>(L, -1, "y", val.linear.y);
        luaU_setfield<double>(L, -1, "z", val.linear.z);
        luaU_setfield<double>(L, -1, "rx", val.angular.x);
        luaU_setfield<double>(L, -1, "ry", val.angular.y);
        luaU_setfield<double>(L, -1, "rz", val.angular.z);
    }
};

#endif
