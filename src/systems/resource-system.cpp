#include "systems/resource-system.hpp"

namespace trillek {
namespace resource {

std::once_flag ResourceMap::only_one;
std::shared_ptr<ResourceMap> ResourceMap::instance = nullptr;
std::map<std::string, unsigned int> ResourceMap::resource_type_id;
std::map<std::string, std::function<std::shared_ptr<resource::ResourceBase>(const std::string& name, const std::vector<Property> &properties)>> ResourceMap::factories;
std::map<unsigned int, std::map<std::string, std::shared_ptr<resource::ResourceBase>>> ResourceMap::resources;

bool ResourceMap::Serialize(rapidjson::Document& document) {
    rapidjson::Value resource_node(rapidjson::kObjectType);

    document.AddMember("resources", resource_node, document.GetAllocator());
    return true;
}

// "resources": {
//   "type" : {
//     "name": {
//       "prop_name_string": "prop_value",
//       "prop_name_int": 1,
//       "prop_name_double": 1.0f,
//       "prop_name_bool": False
//     }
//   }
// }
bool ResourceMap::Parse(rapidjson::Value& node) {
    if (node.IsObject()) {
        // Iterate over the resource types.
        for (auto type_itr = node.MemberBegin(); type_itr != node.MemberEnd(); ++type_itr) {
            std::string resource_type(type_itr->name.GetString(), type_itr->name.GetStringLength());

            if (type_itr->value.IsObject()) {
                // Iterate of the individual resources.
                for (auto res_itr = type_itr->value.MemberBegin(); res_itr != type_itr->value.MemberEnd(); ++res_itr) {
                    std::vector<Property> props;
                    std::string resource_name(res_itr->name.GetString(), res_itr->name.GetStringLength());

                    if (res_itr->value.IsObject()) {
                        // Iterate over the resource's properties.
                        for (auto prop_itr = res_itr->value.MemberBegin(); prop_itr != res_itr->value.MemberEnd(); ++prop_itr) {
                            std::string property_name(prop_itr->name.GetString(), prop_itr->name.GetStringLength());

                            if (prop_itr->value.IsString()) {
                                std::string property_value(prop_itr->value.GetString(), prop_itr->value.GetStringLength());
                                Property p(property_name, property_value);
                                props.push_back(p);
                            }
                            else if (prop_itr->value.IsBool()) {
                                bool property_value = prop_itr->value.GetBool();
                                Property p(property_name, property_value);
                                props.push_back(p);
                            }
                            else if (prop_itr->value.IsDouble()) {
                                double property_value = prop_itr->value.GetDouble();
                                Property p(property_name, property_value);
                                props.push_back(p);
                            }
                            else if (prop_itr->value.IsInt()) {
                                int property_value = prop_itr->value.GetInt();
                                Property p(property_name, property_value);
                                props.push_back(p);
                            }
                            else if (prop_itr->value.IsUint()) {
                                unsigned int property_value = prop_itr->value.GetUint();
                                Property p(property_name, property_value);
                                props.push_back(p);
                            }
                        }
                    }

                    if (!Create(resource_type, resource_name, props)) {
                        // TODO: Log an error about creating this resource.
                    }
                }
            }
        }

        return true;
    }

    return false;
}

} // End of resource
} // End of trillek
