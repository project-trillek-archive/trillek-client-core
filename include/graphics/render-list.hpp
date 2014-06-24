#ifndef RENDER_LIST_HPP_INCLUDED
#define RENDER_LIST_HPP_INCLUDED

#include "trillek.hpp"
#include "graphics-base.hpp"
#include "property.hpp"
#include <memory>
#include <list>

namespace trillek {
namespace graphics {

enum class RenderCmd : unsigned int {
    CLEAR_SCREEN = 0,
    MODULE_CMD,
    SCRIPT,
    RENDER,
    SET_PARAM,
    READ_LAYER,
    WRITE_LAYER,
    COPY_LAYER,
    BIND_TEXTURE,
    BIND_SHADER,
};

/**
 * \brief RenderList - a class to manage the order of the scene rendering
 */
class RenderList : public GraphicsBase {
public:
    RenderList() {}
    virtual ~RenderList() {}

    // required to implement
    virtual bool SystemStart(const std::list<Property> &) override { return true; }
    virtual bool SystemReset(const std::list<Property> &) override { return true; }

    virtual bool Serialize(rapidjson::Document& document) override;
    virtual bool Parse(const std::string &object_name, const rapidjson::Value& node) override;

    std::list<std::pair<RenderCmd, std::list<Property>>> render_commands;
};

} // namespace graphics

namespace reflection {

template <> inline const char* GetTypeName<graphics::RenderList>() { return "render-list"; }
template <> inline const unsigned int GetTypeID<graphics::RenderList>() { return 404; }

} // namespace reflection
} // namespace trillek

#endif
