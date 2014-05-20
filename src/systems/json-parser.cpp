#include "systems/json-parser.hpp"
#include "resources/TextFile.h"
#include "systems/ResourceSystem.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

#include <mutex>

namespace trillek {
namespace system {

JSONParser::JSONParser() {
    static std::once_flag only_one;

    std::call_once(only_one, [this] () { RegisterTypes(); } );
}

bool JSONParser::Parse(const std::string& fname) {
    std::vector<Property> props;
    Property p("filename", std::string("assets/tests/sample.json"));
    props.push_back(p);
    this->file = system::ResourceSystem::GetInstance()->Create<trillek::resource::TextFile>("JSON_test", props);

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

void JSONParser::Save(const std::string& out_directory, const std::string& fname) {
    if (fname.length() > 0) {
        rapidjson::Document document;
        document.SetObject();

        for (auto serializer : serializer_types) {
            serializer.second->Serialize(document);
        }

        FILE* file = fopen((out_directory + fname).c_str(), "w");
        rapidjson::FileStream f(file);
        rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
        document.Accept(writer);
        fclose(file);
    }
    else {
        for (auto serializer : serializer_types) {
            rapidjson::Document document;
            document.SetObject();
            serializer.second->Serialize(document);

            FILE* file = fopen((out_directory + serializer.first + ".json").c_str(), "w");
            rapidjson::FileStream f(file);
            rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
            document.Accept(writer);
            fclose(file);
        }
    }
}

std::map<std::string, std::shared_ptr<SerializerBase>> JSONParser::serializer_types;

void JSONParser::RegisterSerializer(std::shared_ptr<SerializerBase> serializer) {
    serializer_types[serializer->GetName()] = serializer;
}

} // End of system
} // End of trillek
