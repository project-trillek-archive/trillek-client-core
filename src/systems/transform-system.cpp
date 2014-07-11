#include "systems/transform-system.hpp"
#include "transform.hpp"

namespace trillek {

std::once_flag TransformMap::only_one;
std::shared_ptr<TransformMap> TransformMap::instance = nullptr;

std::shared_ptr<Transform> TransformMap::GetTransform(const unsigned int entity_id) {
    if (instance->transforms.find(entity_id) != instance->transforms.end()) {
        return instance->transforms[entity_id];
    }

    return nullptr;
}

std::shared_ptr<Transform> TransformMap::AddTransform(const unsigned int entity_id) {
    if (instance->transforms.find(entity_id) == instance->transforms.end()) {
        std::shared_ptr<Transform> transform = std::make_shared<Transform>(entity_id);

        instance->transforms[entity_id] = transform;
    }

    return instance->transforms[entity_id];
}

void TransformMap::RemoveTransform(const unsigned int entity_id) {
    instance->transforms.erase(entity_id);
}

bool TransformMap::Serialize(rapidjson::Document& document) {
    rapidjson::Value transform_node(rapidjson::kObjectType);

    for (auto entity_transform : this->transforms) {
        rapidjson::Value transform_object(rapidjson::kObjectType);

        rapidjson::Value translation_element(rapidjson::kObjectType);
        glm::vec3 translation = entity_transform.second->GetTranslation();
        translation_element.AddMember("x", translation.x, document.GetAllocator());
        translation_element.AddMember("y", translation.y, document.GetAllocator());
        translation_element.AddMember("z", translation.z, document.GetAllocator());
        transform_object.AddMember("position", translation_element, document.GetAllocator());

        rapidjson::Value rotation_element(rapidjson::kObjectType);
        glm::vec3 rotation = entity_transform.second->GetRotation();
        rotation_element.AddMember("radians", true, document.GetAllocator());
        rotation_element.AddMember("x", rotation.x, document.GetAllocator());
        rotation_element.AddMember("y", rotation.y, document.GetAllocator());
        rotation_element.AddMember("z", rotation.z, document.GetAllocator());
        transform_object.AddMember("rotation", rotation_element, document.GetAllocator());

        rapidjson::Value scale_element(rapidjson::kObjectType);
        glm::vec3 scale = entity_transform.second->GetScale();
        scale_element.AddMember("x", scale.x, document.GetAllocator());
        scale_element.AddMember("y", scale.y, document.GetAllocator());
        scale_element.AddMember("z", scale.z, document.GetAllocator());
        transform_object.AddMember("scale", scale_element, document.GetAllocator()); 

        std::string id = std::to_string(entity_transform.first);
        rapidjson::Value entity_id(id.c_str(), id.length(), document.GetAllocator());

        transform_node.AddMember(entity_id, transform_object, document.GetAllocator());
    }

    document.AddMember("transforms", transform_node, document.GetAllocator());

    return true;
}

//  "transforms": {
//      "0": {
//          "position": {
//              "x": "0.0f",
//              "y" : "0.0f",
//              "z" : "0.0f"
//         },
//         "rotation" : {
//              "x": "0.0f",
//              "y" : "0.0f",
//              "z" : "0.0f"
//          },
//          "scale" : {
//              "x": "0.0f",
//              "y" : "0.0f",
//              "z" : "0.0f"
//          }
//      }
//  }
bool TransformMap::Parse(rapidjson::Value& node) {
    if (node.IsObject()) {
        // Iterate over the entity ids.
        for (auto entity_itr = node.MemberBegin(); entity_itr != node.MemberEnd(); ++entity_itr) {
            if (entity_itr->value.IsObject()) {
                unsigned int entity_id = atoi(entity_itr->name.GetString());
                auto entity_transform = AddTransform(entity_id);

                if (entity_itr->value.HasMember("position")) {
                    auto& element = entity_itr->value["position"];

                    double x = 0.0f, y = 0.0f, z = 0.0f;
                    if (element.HasMember("x") && element["x"].IsNumber()) {
                        x = element["x"].GetDouble();
                    }
                    if (element.HasMember("y") && element["y"].IsNumber()) {
                        y = element["y"].GetDouble();
                    }
                    if (element.HasMember("z") && element["z"].IsNumber()) {
                        z = element["z"].GetDouble();
                    }

                    entity_transform->SetTranslation(glm::vec3(x, y, z));
                }
                if (entity_itr->value.HasMember("rotation")) {
                    auto& element = entity_itr->value["rotation"];

                    bool in_radians = false;

                    if (element.HasMember("radians") && element["radians"].IsBool()) {
                        in_radians = element["radians"].GetBool();
                    }

                    double x = 0.0f, y = 0.0f, z = 0.0f;
                    if (element.HasMember("x") && element["x"].IsNumber()) {
                        x = element["x"].GetDouble();
                        if (!in_radians) {
                            x = glm::radians(x);
                        }
                    }
                    if (element.HasMember("y") && element["y"].IsNumber()) {
                        y = element["y"].GetDouble();
                        if (!in_radians) {
                            y = glm::radians(y);
                        }
                    }
                    if (element.HasMember("z") && element["z"].IsNumber()) {
                        z = element["z"].GetDouble();
                        if (!in_radians) {
                            z = glm::radians(z);
                        }
                    }

                    entity_transform->SetRotation(glm::vec3(x, y, z));
                }
                if (entity_itr->value.HasMember("scale")) {
                    auto& element = entity_itr->value["scale"];

                    double x = 0.0f, y = 0.0f, z = 0.0f;
                    if (element.HasMember("x") && element["x"].IsNumber()) {
                        x = element["x"].GetDouble();
                    }
                    if (element.HasMember("y") && element["y"].IsNumber()) {
                        y = element["y"].GetDouble();
                    }
                    if (element.HasMember("z") && element["z"].IsNumber()) {
                        z = element["z"].GetDouble();
                    }

                    entity_transform->SetScale(glm::vec3(x, y, z));
                }
            }
        }

        return true;
    }

    return false;
}

} // End of trillek
