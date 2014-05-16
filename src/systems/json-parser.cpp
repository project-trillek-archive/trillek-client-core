#include "systems/json-parser.hpp"
#include "resources/TextFile.h"
#include "systems/ResourceSystem.h"

namespace trillek {
namespace system {

JSONParser::JSONParser() {
    this->res_sys = resource::ResourceSystem::GetInstance();
    this->res_sys->Register<resource::TextFile>();
}

bool JSONParser::Parse(const std::string& fname) {
    if (!this->res_sys) {
        return false;
    }

    std::vector<Property> props;
    Property p("filename", std::string("assets/tests/sample.json"));
    props.push_back(p);
    this->file = this->res_sys->Create<trillek::resource::TextFile>("JSON_test", props);

    this->document.Parse<0>(this->file->GetText().c_str());
    if (this->document.HasParseError()) {
        return false;
    }

    if (this->document.IsObject()) {
        for (auto itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
            std::string name(itr->name.GetString(), itr->name.GetStringLength());
            if (name == "resources") {
                LoadResources(itr->value);
            }
        }
    }

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
void JSONParser::LoadResources(const rapidjson::Value& values) {
    if (values.IsObject()) {
        // Iterate over the resrouce types.
        for (auto type_itr = values.MemberBegin(); type_itr != values.MemberEnd(); ++type_itr) {
            std::string resource_type(type_itr->name.GetString(), type_itr->name.GetStringLength());
            unsigned int resource_type_id = this->res_sys->GetTypeIDFromName(resource_type);

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

                    this->res_sys->Create(resource_type_id, resource_name, props);
                }
            }
        }
    }
}

} // End of system
} // End of trillek
