#ifndef GRAPHICS_BASE_INCLUDED
#define GRAPHICS_BASE_INCLUDED

#include <list>
#include "property.hpp"
#include <rapidjson/document.h>

namespace trillek {
namespace graphics {

class GraphicsBase {
public:
    GraphicsBase() : initialize_priority(0) {}
    virtual ~GraphicsBase() {}

    /**
     * \brief called when the graphics system starts to initialize objects
     * \param std::list<Property>& information about the graphics settings
     * \return false on errors, true for success
     */
    virtual bool SystemStart(const std::list<Property> &) = 0;

    /**
     * \brief called when the graphics system needs to reinitialize objects
     * \param std::list<Property>& information about the graphics settings
     * \return false on errors, true for success
     */
    virtual bool SystemReset(const std::list<Property> &) = 0;

    /**
     * \brief parse a graphics object from json
     * \param[in] const std::string& object_name the name of the node
     * \param[in] rapidjson::Value& node The node to parse.
     * \return false on errors, true for success
     */
    virtual bool Parse(const std::string &object_name, const rapidjson::Value& node) = 0;

    /**
     * \brief Serialize this graphics object to the provided JSON node.
     *
     * \param[in] rapidjson::Document& document The document to serialize to.
     * \return bool False if an error occured in serializing.
     */
    virtual bool Serialize(rapidjson::Document& document) = 0;

    unsigned int initialize_priority;
};

} // namespace graphics
} // namespace trillek

#endif
