#ifndef LUA_GLM_H_PP
#define LUA_GLM_H_PP

#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include <glm/glm.hpp>

template<>
struct luaU_Impl<glm::vec3> {
    static glm::vec3 luaU_check(lua_State* L, int index) {
        return glm::vec3(
            luaU_getfield<float>(L, index, "x"),
            luaU_getfield<float>(L, index, "y"),
            luaU_getfield<float>(L, index, "z"));
    }

    static glm::vec3 luaU_to(lua_State* L, int index) {
        return glm::vec3(
            luaU_getfield<float>(L, index, "x"),
            luaU_getfield<float>(L, index, "y"),
            luaU_getfield<float>(L, index, "z"));
    }

    static void luaU_push(lua_State* L, const glm::vec3& val) {
        lua_newtable(L);
        luaU_setfield<float>(L, -1, "x", val.x);
        luaU_setfield<float>(L, -1, "y", val.y);
        luaU_setfield<float>(L, -1, "z", val.z);
    }
};

#endif
