#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "lua/lua_glm.hpp"

#include "transform.hpp"
#include "systems/transform-system.hpp"

namespace trillek {
namespace script {

int Traansform_get(lua_State* L) {
    const int entity_id = luaL_checkinteger(L, 1);
    auto transform = TransformMap::GetTransform(entity_id);
    if (!transform) {
        transform = TransformMap::AddTransform(entity_id);
    }
    luaW_push<Transform>(L, transform.get());
    return 1;
}

int Transform_translate(lua_State* L) {
    Transform* trasnform = luaW_check<Transform>(L, 1);
    const double x = luaL_checknumber(L, 2);
    const double y = luaL_checknumber(L, 3);
    const double z = luaL_checknumber(L, 4);
    trasnform->Translate(glm::vec3(x, y, z));
    trasnform->MarkAsModified();
    return 0;
}

int Transform_rotate(lua_State* L) {
    Transform* trasnform = luaW_check<Transform>(L, 1);
    const double x = luaL_checknumber(L, 2);
    const double y = luaL_checknumber(L, 3);
    const double z = luaL_checknumber(L, 4);
    trasnform->Rotate(glm::vec3(x, y, z));
    trasnform->MarkAsModified();
    return 0;
}

int Transform_scale(lua_State* L) {
    Transform* trasnform = luaW_check<Transform>(L, 1);
    const double x = luaL_checknumber(L, 2);
    const double y = luaL_checknumber(L, 3);
    const double z = luaL_checknumber(L, 4);
    trasnform->Scale(glm::vec3(x, y, z));
    trasnform->MarkAsModified();
    return 0;
}

int Transform_settranslation(lua_State* L) {
    Transform* trasnform = luaW_check<Transform>(L, 1);
    const double x = luaL_checknumber(L, 2);
    const double y = luaL_checknumber(L, 3);
    const double z = luaL_checknumber(L, 4);
    trasnform->SetTranslation(glm::vec3(x, y, z));
    trasnform->MarkAsModified();
    return 0;
}

int Transform_setrotation(lua_State* L) {
    Transform* trasnform = luaW_check<Transform>(L, 1);
    const double x = luaL_checknumber(L, 2);
    const double y = luaL_checknumber(L, 3);
    const double z = luaL_checknumber(L, 4);
    trasnform->SetRotation(glm::vec3(x, y, z));
    trasnform->MarkAsModified();
    return 0;
}

int Transform_setscale(lua_State* L) {
    Transform* trasnform = luaW_check<Transform>(L, 1);
    const double x = luaL_checknumber(L, 2);
    const double y = luaL_checknumber(L, 3);
    const double z = luaL_checknumber(L, 4);
    trasnform->SetScale(glm::vec3(x, y, z));
    trasnform->MarkAsModified();
    return 0;
}

static luaL_Reg Transform_table[] =
{
    { "Get", Traansform_get },
    { NULL, NULL }
};

static luaL_Reg Transform_metatable[] =
{
    { "translate", Transform_translate },
    { "rotate", Transform_rotate },
    { "scale", Transform_scale },
    { "set_translation", Transform_settranslation },
    { "set_rotation", Transform_setrotation },
    { "set_scale", Transform_setscale },
    { "get_translation", luaU_get<Transform, glm::vec3, &Transform::GetTranslation> },
    { "get_rotation", luaU_get<Transform, glm::vec3, &Transform::GetRotation> },
    { "get_scale", luaU_get<Transform, glm::vec3, &Transform::GetScale> },
};

int luaopen_Transform(lua_State* L) {
    luaW_register<Transform>(L,
        "Transform",
        Transform_table,
        Transform_metatable,
        nullptr // If your class has a default constructor you can omit this argument,
        // LuaWrapper will generate a default allocator for you.
        );
    return 1;
}

} // End of script
} // End of trillek
