#ifndef JSON_PARSER_HPP_INCLUDED
#define JSON_PARSER_HPP_INCLUDED

#include <memory>
#include <rapidjson/document.h>
#include <string>

namespace trillek {
namespace resource {

class TextFile;
class ResourceSystem;

} // End of resource

namespace system {

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
    std::shared_ptr<trillek::resource::ResourceSystem> res_sys; // Reference to the resource system (obtained in ctor).
};

} // End of system
} // End of trillek

#endif
