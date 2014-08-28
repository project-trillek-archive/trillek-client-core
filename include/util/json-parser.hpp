#ifndef JSON_PARSER_HPP_INCLUDED
#define JSON_PARSER_HPP_INCLUDED

#include <memory>
#include <map>
#include <list>
#include <rapidjson/document.h>
#include <string>

namespace trillek {
namespace resource {

class TextFile;

} // End of resource

namespace util {

std::string MakeString(const rapidjson::Value& v);

class Parser {
protected:
    Parser(std::string name) : node_type_name(name) { }
public:
    /**
     * \brief Serialize this object to the provided JSON node.
     *
     * \param[in] rapidjson::Document& document The document to serialize to.
     * \return bool False if an error occured in serializing.
     */
    virtual bool Serialize(rapidjson::Document& document) = 0;

    /**
     * \brief Parses this object from the provided JSON node.
     *
     * \param[in] rapidjson::Value& node The node to parse.
     * \return bool False if an error occured in parsing.
     */
    virtual bool Parse(rapidjson::Value& node) = 0;

    /**
     * \brief Gets the name of this node type.
     *
     * This type name matches the one the top-level JSON node.
     * \return std::string Node type name for this parser.
     */
    std::string GetNodeTypeName() {
        return this->node_type_name;
    }
private:
    std::string node_type_name;
};

struct JSONDocument {
    std::shared_ptr<resource::TextFile> file; // The loaded TextFile. Usefull if the file is reloaded.
    rapidjson::Document document; // Currently parsed document.
};

class JSONPasrser {
public:
    JSONPasrser();

    /**
     * \brief Parses a JSON file with the given filename.
     *
     * A TextFile resource is created with the filename if it doesn't exist already.
     * \param[in] const std::string& fname The filename of the JSON file to load.
     * \return bool True if parsing was successfully.
     */
    bool Parse(const std::string& fname);

    /**
     * \brief Serializes each parser node type to disk.
     *
     * If there isn't a supplied filename then each type is serialized as "node_type_name.json
     * \param[in] const std::string& out_directory The directory in which to place the file(s).
     * \param[in] const std::string& fname The filename of where to serialize the JSON.
     * \return void
     */
    void Serialize(const std::string& out_directory, const std::string& fname, std::shared_ptr<Parser> parser);

    /**
     * \brief Registers a serializer type.
     *
     * This method will call GetName() on the serializer to get its type name. If a type
     * with the same name exists it is overridden.
     * \param[in] std::shared_ptr<Parser> serializer The serializer to register.
     * \return void
     */
    static void RegisterParser(std::shared_ptr<Parser> parser);

    /**
     * \brief Registers all parsers as defined in the method body.
     *
     * This function is defined in a separate source file to reduce compile times.
     * Place each parser to be register at compile time in the method body.
     * Interally it just calls the tempalte method RegisterParser().
     * \return void
     */
    static void RegisterTypes();
private:
    std::list<std::shared_ptr<JSONDocument>> documents;
    static std::map<std::string, std::shared_ptr<Parser>> parsers; // Mapping of node_type_name to parser
};

} // End of json
} // End of trillek

#endif
