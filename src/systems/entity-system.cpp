#include "systems/entity-system.hpp"

namespace trillek {

std::once_flag EntityMap::only_one;
std::shared_ptr<EntityMap> EntityMap::instance = nullptr;
std::map<std::string, unsigned int> EntityMap::component_type_id;
std::map<unsigned int, std::function<std::shared_ptr<ComponentBase>(const unsigned int,
    const std::vector<Property> &properties)>> EntityMap::factories;
std::map<unsigned int, std::map<unsigned int, std::shared_ptr<ComponentBase>>> EntityMap::components;
std::map<unsigned int, SystemBase*> EntityMap::systems;

bool EntityMap::Serialize(rapidjson::Document& document) {
    rapidjson::Value component_node(rapidjson::kObjectType);

    document.AddMember("entities", component_node, document.GetAllocator());
    return true;
}

// "entities": {
//   "name" : {
//     "id": xx,
//      component {
//       "prop_name_string": "prop_value",
//       "prop_name_int": 1,
//       "prop_name_double": 1.0f,
//       "prop_name_bool": False
//     }
//   }
// }
bool EntityMap::DeSerialize(rapidjson::Value& node) {
    if (node.IsObject()) {
        // Iterate over the resrouce types.
        for (auto entity_itr = node.MemberBegin(); entity_itr != node.MemberEnd(); ++entity_itr) {
            if (entity_itr->value.IsObject()) {
                std::string entity_name(entity_itr->name.GetString(), entity_itr->name.GetStringLength());
                unsigned int entity_id = 0;

                for (auto entity_property_itr = entity_itr->value.MemberBegin();
                    entity_property_itr != entity_itr->value.MemberEnd(); ++entity_property_itr) {
                    std::string entity_property_name(entity_property_itr->name.GetString(), entity_property_itr->name.GetStringLength());
                    if (entity_property_name == "id") {
                        entity_id = atoi(entity_itr->name.GetString());
                    }
                    else {
                        std::vector<Property> props;
                        props.push_back(Property("entity_id", entity_id));
                        unsigned int component_type_id = GetTypeIDFromName(entity_property_name);
                        if (entity_property_itr->value.IsObject()) {
                            for (auto component_property_itr = entity_property_itr->value.MemberBegin(); 
                                component_property_itr != entity_property_itr->value.MemberEnd(); ++component_property_itr) {
                                std::string component_property_name(component_property_itr->name.GetString(),
                                    component_property_itr->name.GetStringLength());

                                if (component_property_itr->value.IsString()) {
                                    std::string property_value(component_property_itr->value.GetString(), 
                                        component_property_itr->value.GetStringLength());
                                    Property p(component_property_name, property_value);
                                    props.push_back(p);
                                }
                                else if (component_property_itr->value.IsBool()) {
                                    bool property_value = component_property_itr->value.GetBool();
                                    Property p(component_property_name, property_value);
                                    props.push_back(p);
                                }
                                else if (component_property_itr->value.IsDouble()) {
                                    double property_value = component_property_itr->value.GetDouble();
                                    Property p(component_property_name, property_value);
                                    props.push_back(p);
                                }
                                else if (component_property_itr->value.IsInt()) {
                                    int property_value = component_property_itr->value.GetInt();
                                    Property p(component_property_name, property_value);
                                    props.push_back(p);
                                }
                                else if (component_property_itr->value.IsUint()) {
                                    unsigned int property_value = component_property_itr->value.GetUint();
                                    Property p(component_property_name, property_value);
                                    props.push_back(p);
                                }
                            }

                            if (!Create(component_type_id, entity_id, props)) {
                                // TODO: Log an error about creating this component.
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    return false;
}

} // End of trillek
