#include "systems/lua-system.hpp"

#include <luawrapper/luawrapper.hpp>
#include <luawrapper/luawrapperutil.hpp>

#include "lua/lua_glm.hpp"

#include "transform.hpp"
#include "trillek-game.hpp"
#include "components/component.hpp"
#include "components/shared-component.hpp"
#include "systems/transform-system.hpp"

namespace trillek {
namespace script {

int Traansform_get(lua_State* L) {
    const int entity_id = luaL_checkinteger(L, 1);
    auto transform = component::GetConstSharedPtr<component::Component::GraphicTransform>(entity_id);
    if (!transform) {
        return 0;
    }
    luaW_push<Transform>(L, const_cast<Transform*>(transform.get()));
    return 1;
}

int ComputeVelocityVector(lua_State* L) {
    Transform* transform = luaW_check<Transform>(L, 1);
    glm::vec3 speed = luaU_check<glm::vec3>(L, 2);
    glm::vec3 velocity_vector = transform->GetOrientation() * speed;
    luaU_push<glm::vec3>(L, velocity_vector);
    return 1;
}

static luaL_Reg Transform_table[] =
{
    { "Get", Traansform_get },
    { nullptr, nullptr } // table end marker
};

static luaL_Reg Transform_metatable[] =
{
    { "translate", luaU_set<Transform, glm::vec3, &Transform::Translate> },
    { "rotate", luaU_set<Transform, glm::vec3, &Transform::Rotate> },
    { "scale", luaU_set<Transform, glm::vec3, &Transform::Scale> },
    { "set_translation", luaU_set<Transform, glm::vec3, &Transform::SetTranslation> },
    { "set_rotation", luaU_set<Transform, glm::vec3, &Transform::SetRotation> },
    { "set_scale", luaU_set<Transform, glm::vec3, &Transform::SetScale> },
    { "get_translation", luaU_get<Transform, glm::vec3, &Transform::GetTranslation> },
    { "get_rotation", luaU_get<Transform, glm::vec3, &Transform::GetRotation> },
    { "get_scale", luaU_get<Transform, glm::vec3, &Transform::GetScale> },
    { "compute_velocity_vector", &ComputeVelocityVector },
    { nullptr, nullptr } // table end marker
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
