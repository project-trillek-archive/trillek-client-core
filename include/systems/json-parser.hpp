#ifndef JSON_PARSER_HPP_INCLUDED
#define JSON_PARSER_HPP_INCLUDED

#include <memory>
#include <map>
#include <rapidjson/document.h>
#include <string>

namespace trillek {
namespace resource {

class TextFile;
class ResourceSystem;

} // End of resource

namespace system {

class SerializerBase {
protected:
    SerializerBase(std::string name) : name(name) { }
public:
    virtual bool Serialize(rapidjson::Value& node) = 0;
    virtual bool DeSerialize(rapidjson::Value& node) = 0;

    std::string GetName() {
        return this->name;
    }
private:
    std::string name;
};

class JSONParser {
public:
    JSONParser();

    /**
    * \brief Parses a JSON file with the given filename.
    *
    * There is a TextFile resource created with the filename if it doesn't exist already.
    * \param[in] const std::string& fname The filename of the JSON file to load.
    * \return bool True if parsing was successfully.
    */
    bool Parse(const std::string& fname);

    /**
    * \brief Registers a serializer type.
    *
    * This method will call GetName() on the serializer to get its type name. If a type
    * with the same name exists it is overridden.
    * \param[in] std::shared_ptr<SerializerBase> serializer The serializer to register.
    * \return void
    */
    static void RegisterSerializer(std::shared_ptr<SerializerBase> serializer);
private:
    /**
    * \brief Parses and load the resources for a JSON node.
    *
    * \param[in] const rapidjson::Value& values The node to parse and load resource from.
    * \return void
    */
    void LoadResources(const rapidjson::Value& values);

    std::shared_ptr<resource::TextFile> file; // The loaded TextFile. Usefull if the file is reloaded.
    rapidjson::Document document; // Currently parsed document.
    static std::map<std::string, std::shared_ptr<SerializerBase>> serializer_types;
};

} // End of system
} // End of trillek

#endif
