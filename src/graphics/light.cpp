
#include "graphics/light.hpp"
#include "type-id.hpp"

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
            else if(vec_itr->Is<int>()) {
                radius = static_cast<float>(vec_itr->Get<int>());
            }
            else if(vec_itr->Is<long>()) {
                radius = static_cast<float>(vec_itr->Get<long>());
            }
            else if(vec_itr->Is<float>()) {
                radius = vec_itr->Get<float>();
            }
        }
        else if(vec_itr->GetName() == "color" && vec_itr->Is<glm::vec3>()) {
            color = vec_itr->Get<glm::vec3>();
        }
    }
    light_props.push_back(Property("radius", radius));

    return true;
}

} // namespace graphics
} // namespace trillek
