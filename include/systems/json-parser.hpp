#ifndef JSON_PARSER_HPP_INCLUDED
#define JSON_PARSER_HPP_INCLUDED

#include <memory>
#include <map>
#include <rapidjson/document.h>
#include <string>

namespace trillek {
namespace resource {

class TextFile;
class System;

} // End of system

namespace json {

class SerializerBase {
protected:
    SerializerBase(std::string name) : name(name) { }
public:
    /**
    * \brief Serialize this object to the provided JSON node.
    *
    * \param[in] rapidjson::Document& document The document to serialize to.
    * \return bool False if an error occured in serializing.
    */
    virtual bool Serialize(rapidjson::Document& document) = 0;

    /**
    * \brief Deserialize this object from the provided JSON node.
    *
    * \param[in] rapidjson::Value& node The node to deserialize from.
    * \return bool False if an error occured in deserializing.
    */
    virtual bool DeSerialize(rapidjson::Value& node) = 0;

    /**
    * \brief Gets the name of this serializable type.
    *
    * This type name matches the one the top-level JSON node.
    * \return std::string The name of this serializable type.
    */
    std::string GetName() {
        return this->name;
    }
private:
    std::string name;
};

class System {
public:
    System();

    /**
    * \brief Parses a JSON file with the given filename.
    *
    * There is a TextFile resource created with the filename if it doesn't exist already.
    * \param[in] const std::string& fname The filename of the JSON file to load.
    * \return bool True if parsing was successfully.
    */
    bool Parse(const std::string& fname);

    /**
    * \brief Saves each serializer type to disk.
    *
    * If there isn't a supplied filename then each type is saved as "serializer_name.json
    * \param[in] const std::string& out_directory The directory in which to place the file(s).
    * \param[in] const std::string& fname The filename of where to save the JSON.
    * \return void
    */
    void Save(const std::string& out_directory, const std::string& fname);

    /**
    * \brief Registers a serializer type.
    *
    * This method will call GetName() on the serializer to get its type name. If a type
    * with the same name exists it is overridden.
    * \param[in] std::shared_ptr<SerializerBase> serializer The serializer to register.
    * \return void
    */
    static void RegisterSerializer(std::shared_ptr<SerializerBase> serializer);

    /**
    * \brief Registers all serializer types as defined in the function body.
    *
    * This function is defined in a separate source file to reduce compile times.
    * This function is coupled to each serializer type, and all serializer types
    * known at compile time should be registered via the function body.
    * Interally it just calls the tempalte method RegisterSerializer().
    * \return void
    */
    static void RegisterTypes();
private:
    std::shared_ptr<resource::TextFile> file; // The loaded TextFile. Usefull if the file is reloaded.
    rapidjson::Document document; // Currently parsed document.
    static std::map<std::string, std::shared_ptr<SerializerBase>> serializer_types;
};

} // End of json
} // End of trillek

#endif
