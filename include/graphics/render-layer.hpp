#ifndef RENDER_LAYER_HPP_INCLUDED
#define RENDER_LAYER_HPP_INCLUDED

#include "opengl.hpp"
#include "trillek.hpp"
#include "graphics-base.hpp"
#include <rapidjson/document.h>
#include "texture.hpp"
#include <vector>
#include <memory>

namespace trillek {
namespace graphics {

/**
 * \brief A class to manage a color or depth output from a render pass
 */
class RenderAttachment : public GraphicsBase {
public:
    RenderAttachment();
    ~RenderAttachment();
    RenderAttachment(const RenderAttachment &) = delete;
    RenderAttachment& operator=(const RenderAttachment &) = delete;
    RenderAttachment(RenderAttachment &&that);
    RenderAttachment& operator=(RenderAttachment &&that);

    /**
     * \brief parse a RenderAttachment from json
     * \return false on errors, true for success
     */
    bool Parse(rapidjson::Value& node);
private:
    GLuint renderbuf;
    bool multisample;
    GLenum attachtarget;
    std::string texturename;
    std::weak_ptr<Texture> texture;
};

/**
 * \brief A class to handle rendering a pass to a framebuffer
 */
class RenderLayer : public GraphicsBase {
public:
    RenderLayer();
    virtual ~RenderLayer();

    /**
     * \brief parse a RenderLayer from json
     * \return false on errors, true for success
     */
    bool Parse(rapidjson::Value& node);
private:

    GLuint fbo_id;
    std::vector<std::weak_ptr<RenderAttachment>> attachments;
};

} // namespace graphics

namespace reflection {

template <> inline const char* GetTypeName<graphics::RenderAttachment>() { return "attachment"; }
template <> inline const unsigned int GetTypeID<graphics::RenderAttachment>() { return 402; }
template <> inline const char* GetTypeName<graphics::RenderLayer>() { return "render"; }
template <> inline const unsigned int GetTypeID<graphics::RenderLayer>() { return 403; }

} // namespace reflection
} // namespace trillek

#endif
