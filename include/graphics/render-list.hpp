#ifndef RENDER_LIST_HPP_INCLUDED
#define RENDER_LIST_HPP_INCLUDED

#include "trillek.hpp"
#include "graphics-base.hpp"
#include "property.hpp"
#include "container.hpp"
#include <memory>
#include <string>
#include <list>
#include <tuple>

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
    SET_RENDER_LAYER,
    COPY_LAYER,
    BIND_TEXTURE,
    BIND_SHADER,
};

class RenderCommandItem {
public:
    RenderCommandItem(RenderCmd c, Container &&cv, std::list<Property> &&prop) {
        cmd = c;
        resolved = false;
        cmdvalue = std::move(cv);
        load_properties = std::move(prop);
    }
    ~RenderCommandItem() {}

    RenderCommandItem(const RenderCommandItem&) = delete;
    RenderCommandItem& operator=(const RenderCommandItem&) = delete;

    RenderCommandItem(RenderCommandItem&&) = default;
    RenderCommandItem& operator=(RenderCommandItem&&) = default;

    RenderCmd cmd;
    Container cmdvalue;
    bool resolved;
    std::list<Container> run_values;
    std::list<Property> load_properties;
};
/**
 * \brief RenderList - a class to manage the order of the scene rendering
 */
class RenderList : public GraphicsBase {
public:
    RenderList() { initialize_priority = 2; }
    virtual ~RenderList() {}

    // required to implement
    virtual bool SystemStart(const std::list<Property> &) override;
    virtual bool SystemReset(const std::list<Property> &) override;

    virtual bool Serialize(rapidjson::Document& document) override;
    virtual bool Parse(const std::string &object_name, const rapidjson::Value& node) override;

    std::list<RenderCommandItem> render_commands;
};

} // namespace graphics

namespace reflection {

template <> inline const char* GetTypeName<graphics::RenderList>() { return "render-list"; }
template <> inline const unsigned int GetTypeID<graphics::RenderList>() { return 404; }

} // namespace reflection
} // namespace trillek

#endif
