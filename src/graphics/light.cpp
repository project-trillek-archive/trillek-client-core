
#include "graphics/light.hpp"
#include "type-id.hpp"
#include "trillek-game.hpp"
#include "systems/graphics.hpp"

namespace trillek {
namespace graphics {

bool LightBase::Initialize(const std::vector<Property> &properties) {

    color = glm::vec3(1,1,1);
    float radius = 200.0f;

    for(auto vec_itr = properties.begin(); vec_itr != properties.end(); vec_itr++) {
        if(vec_itr->GetName() == "enabled") {
            this->enabled = vec_itr->Get<bool>();
        }
        else if(vec_itr->GetName() == "radius") {
            if(vec_itr->Is<double>()) {
                radius = vec_itr->Get<double>();
            }
            else if(vec_itr->Is<int32_t>()) {
                radius = static_cast<float>(vec_itr->Get<int32_t>());
            }
            else if(vec_itr->Is<int64_t>()) {
                radius = static_cast<float>(vec_itr->Get<int64_t>());
            }
            else if(vec_itr->Is<float>()) {
                radius = vec_itr->Get<float>();
            }
        }
        else if(vec_itr->GetName() == "color" && vec_itr->Is<glm::vec3>()) {
            color = vec_itr->Get<glm::vec3>();
        }
        else if(vec_itr->GetName() == "shadow" && vec_itr->Is<std::string>()) {
            light_props.push_back(Property("shadow", vec_itr->Get<std::string>()));
            shadows = true;
        }
    }
    light_props.push_back(Property("radius", radius));

    return true;
}

} // namespace graphics
} // namespace trillek
