#include "util/json-parser.hpp"
#include "resources/text-file.hpp"
#include "systems/resource-system.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"

#include <mutex>

namespace trillek {
namespace util {

std::string MakeString(const rapidjson::Value& v) {
    return std::string(v.GetString(), v.GetStringLength());
}

JSONPasrser::JSONPasrser() {
    static std::once_flag only_one;

    std::call_once(only_one, [this] () { RegisterTypes(); } );
}

bool JSONPasrser::Parse(const std::string& fname) {
    std::vector<Property> props;
    Property p("filename", fname);
    props.push_back(p);
    this->file = resource::ResourceMap::Create<trillek::resource::TextFile>("JSON_test", props);

    this->document.Parse<0>(this->file->GetText().c_str());
    if (this->document.HasParseError()) {
        return false;
    }

    if (this->document.IsObject()) {
        for (auto itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
            std::string name(itr->name.GetString(), itr->name.GetStringLength());
            if (this->parsers.find(name) != this->parsers.end()) {
                this->parsers[name]->Parse(itr->value);
            }
        }
    }

    return true;
}

void JSONPasrser::Serialize(const std::string& out_directory, const std::string& fname) {
    if (fname.length() > 0) {
        rapidjson::Document document;
        document.SetObject();

        for (auto serializer : parsers) {
            serializer.second->Serialize(document);
        }

        FILE* file = fopen((out_directory + fname).c_str(), "w");
        rapidjson::FileStream f(file);
        rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
        document.Accept(writer);
        fclose(file);
    }
    else {
        for (auto serializer : parsers) {
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

std::map<std::string, std::shared_ptr<Parser>> JSONPasrser::parsers;

void JSONPasrser::RegisterParser(std::shared_ptr<Parser> parser) {
    parsers[parser->GetNodeTypeName()] = parser;
}

} // End of json
} // End of trillek
