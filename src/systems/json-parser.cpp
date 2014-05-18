#include "systems/json-parser.hpp"
#include "resources/TextFile.h"
#include "systems/ResourceSystem.h"

namespace trillek {
namespace system {

JSONParser::JSONParser() { }

bool JSONParser::Parse(const std::string& fname) {
    std::vector<Property> props;
    Property p("filename", std::string("assets/tests/sample.json"));
    props.push_back(p);
    this->file = resource::ResourceSystem::GetInstance()->Create<trillek::resource::TextFile>("JSON_test", props);

    this->document.Parse<0>(this->file->GetText().c_str());
    if (this->document.HasParseError()) {
        return false;
    }

    if (this->document.IsObject()) {
        for (auto itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
            std::string name(itr->name.GetString(), itr->name.GetStringLength());
            if (this->serializer_types.find(name) != this->serializer_types.end()) {
                this->serializer_types[name]->DeSerialize(itr->value);
            }
        }
    }

    return true;
}

std::map<std::string, std::shared_ptr<SerializerBase>> JSONParser::serializer_types;

void JSONParser::RegisterSerializer(std::shared_ptr<SerializerBase> serializer) {
    serializer_types[serializer->GetName()] = serializer;
}

} // End of system
} // End of trillek
