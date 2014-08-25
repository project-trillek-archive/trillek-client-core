#include "util/json-parser.hpp"
#include "resources/text-file.hpp"
#include "systems/resource-system.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filestream.h"
#include "logging.hpp"

#include <mutex>
#include <iostream>

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

    auto file = resource::ResourceMap::Create<resource::TextFile>(fname, props);

    if (!file) {
        LOGMSG(ERROR) << "Error parsing: " << fname;
        return false;
    }

    std::shared_ptr<JSONDocument> doc;
    for (auto temp_doc : this->documents) {
        if (temp_doc->file == file) {
            doc = temp_doc;
        }
    }

    if (doc == nullptr) {
        doc = std::make_shared<JSONDocument>();
        doc->file = file;
        this->documents.push_back(doc);
    }

    doc->document.Parse<0>(doc->file->GetText().c_str());
    if (doc->document.HasParseError()) {
        LOGMSG(ERROR) << "JSON error at offset " << doc->document.GetErrorOffset() << ": " << doc->document.GetParseError();
        return false;
    }

    if (doc->document.IsObject()) {
        for (auto itr = doc->document.MemberBegin(); itr != doc->document.MemberEnd(); ++itr) {
            std::string name(itr->name.GetString(), itr->name.GetStringLength());
            if (itr->value.IsString()) {
                std::string value = MakeString(itr->value);
                if (value.find("@") != std::string::npos) {
                    std::string file_path;

                    if (fname.find("/") != std::string::npos) {
                        file_path = fname.substr(0, fname.find_last_of("/") + 1);
                    }
                    else if (fname.find("\"") != std::string::npos) {
                        file_path = fname.substr(0, fname.find_last_of("\"") + 1);
                    }

                    value = file_path + value.substr(1);
                    Parse(value);
                }
            }
            else if (itr->value.IsObject() || itr->value.IsArray()) {
                if (this->parsers.find(name) != this->parsers.end()) {
                    this->parsers[name]->Parse(itr->value);
                }
            }
        }
    }

    return true;
}

void JSONPasrser::Serialize(const std::string& out_directory, const std::string& fname, std::shared_ptr<Parser> parser) {
    if (fname.length() > 0) {
        rapidjson::Document document;
        document.SetObject();

        if (parser) {
            parser->Serialize(document);
        }
        else {
            for (auto serializer : parsers) {
                serializer.second->Serialize(document);
            }
        }

        FILE* file = fopen((out_directory + fname).c_str(), "w");
        rapidjson::FileStream f(file);
        rapidjson::PrettyWriter<rapidjson::FileStream> writer(f);
        document.Accept(writer);
        fclose(file);
    }
    else {

        if (parser) {
            rapidjson::Document document;
            document.SetObject();
            parser->Serialize(document);

            FILE* file = fopen((out_directory + fname + ".json").c_str(), "w");
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
}

std::map<std::string, Parser*> JSONPasrser::parsers;

void JSONPasrser::RegisterParser(std::shared_ptr<Parser> parser) {
    parsers[parser->GetNodeTypeName()] = parser.get();
}

void JSONPasrser::RegisterParser(Parser* parser) {
    parsers[parser->GetNodeTypeName()] = parser;
}

} // End of json
} // End of trillek
