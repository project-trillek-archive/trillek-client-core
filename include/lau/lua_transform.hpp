extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lauxlib.h"
}
#include <luawrapper/luawrapper.hpp>

#include "transform.hpp"
#include "systems/transform-system.hpp"

namespace trillek {
namespace script {

Transform* Traansform_get(lua_State* L) {
    const int entity_id = luaL_checkinteger(L, 1);
    auto transform = TransformMap::GetTransform(entity_id);
    if (!transform) {
        transform = TransformMap::AddTransform(entity_id);
    }
    return transform.get();
}

int Transform_translate(lua_State* L) {
    Transform* trasnform = luaW_check<Transform>(L, 1);
    const int x = luaL_checkinteger(L, 2);
    const int y = luaL_checkinteger(L, 3);
    const int z = luaL_checkinteger(L, 4);
    trasnform->Translate(glm::vec3(x, y, z));
    return 0;
}

static luaL_Reg Transform_table[] =
{
    { NULL, NULL }
};

static luaL_Reg Transform_metatable[] =
{
    { "translate", Transform_translate },
    { NULL, NULL }
};

int luaopen_Transform(lua_State* L) {
    luaW_register<Transform>(L,
        "Transform",
        Transform_table,
        Transform_metatable,
        Traansform_get // If your class has a default constructor you can omit this argument,
        // LuaWrapper will generate a default allocator for you.
        );
    return 1;
}

} // End of script
} // End of trillek
