#ifndef RENDER_LAYER_HPP_INCLUDED
#define RENDER_LAYER_HPP_INCLUDED

#include "opengl.hpp"
#include "trillek.hpp"
#include "type-id.hpp"
#include "graphics-base.hpp"
#include <rapidjson/document.h>
#include <vector>
#include <memory>

namespace trillek {
namespace graphics {

class Texture;

struct ViewRect{
    ViewRect() {}
    ViewRect(GLuint x0, GLuint y0, GLuint x1, GLuint y1) :
        x(x0), y(y0), z(x1), w(y1) { }
    GLuint x;
    GLuint y;
    GLuint z;
    GLuint w;
};

/**
 * \brief A class to manage a color or depth output from a render pass
 */
class RenderAttachment : public GraphicsBase {
public:
    RenderAttachment();
    virtual ~RenderAttachment();
    RenderAttachment(const RenderAttachment &) = delete;
    RenderAttachment& operator=(const RenderAttachment &) = delete;
    RenderAttachment(RenderAttachment &&that);
    RenderAttachment& operator=(RenderAttachment &&that);

    virtual bool SystemStart(const std::list<Property> &);
    virtual bool SystemReset(const std::list<Property> &);

    /**
     * \brief parse an attachment from json
     * \param[in] const std::string& object_name the name of the node
     * \param[in] rapidjson::Value& node The node to parse.
     * \return false on errors, true for success
     */
    virtual bool Parse(const std::string &object_name, const rapidjson::Value& node);

    /**
     * \brief Serialize this attachment to the provided JSON node.
     *
     * \param[in] rapidjson::Document& document The document to serialize to.
     * \return bool False if an error occured in serializing.
     */
    virtual bool Serialize(rapidjson::Document& document);

    void Generate(int width, int height, int samplecount);
    void Destroy();
    void BindTexture();
    void AttachToFBO();

    bool NeedsClear() const { return clearonuse; }
    void Clear();

    bool IsColor() const {
        return (attachtarget == GL_COLOR_ATTACHMENT0);
    }
    bool IsCustomSize() const { return customsize; }

    GLenum GetAttach() const {
        if(attachtarget == GL_COLOR_ATTACHMENT0) {
            return GL_COLOR_ATTACHMENT0 + outputnumber;
        }
        return attachtarget;
    }
private:
    GLuint renderbuf;
    bool multisample;
    bool multisample_texture;
    bool clearonuse;
    bool customsize;
    bool shadowcompare;
    float clearvalues[4];
    int clearstencil;
    unsigned int width;
    unsigned int height;
    GLenum attachtarget;
    int outputnumber;
    std::string texturename;
    std::shared_ptr<Texture> texture;
};

/**
 * \brief A class to handle rendering a pass to a framebuffer
 */
class RenderLayer : public GraphicsBase {
public:
    RenderLayer();
    virtual ~RenderLayer();

    virtual bool SystemStart(const std::list<Property> &);
    virtual bool SystemReset(const std::list<Property> &);

    /**
     * \brief parse a RenderLayer from json
     * \param[in] const std::string& object_name the name of the node
     * \param[in] rapidjson::Value& node The node to parse.
     * \return false on errors, true for success
     */
    virtual bool Parse(const std::string &object_name, const rapidjson::Value& node);

    /**
     * \brief Serialize this RenderLayer to the provided JSON node.
     *
     * \param[in] rapidjson::Document& document The document to serialize to.
     * \return bool False if an error occured in serializing.
     */
    virtual bool Serialize(rapidjson::Document& document);

    void Generate();
    void Destroy();

    static void UnbindFromRead();
    static void UnbindFromWrite();
    static void UnbindFromAll();
    void BindToRender() const;
    void BindToRead() const;
    void BindToWrite() const;
    void BindTextures() const;
    bool IsCustomSize() const { return customsize; }

    void GetRect(ViewRect& vr) {
        vr.x = 0;
        vr.y = 0;
        vr.z = width;
        vr.w = height;
    }

private:

    bool clearany;
    GLuint clearbits;
    bool clearhighbuffers;
    GLuint fbo_id;
    bool customsize;
    unsigned int width;
    unsigned int height;
    std::unique_ptr<GLenum> draworder;
    GLuint drawcount;
    std::vector<std::string> attachmentnames;
    std::vector<std::shared_ptr<RenderAttachment>> attachments;
};

} // namespace graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NAME(graphics::RenderAttachment, "attachment", 403)
TRILLEK_MAKE_IDTYPE_NAME(graphics::RenderLayer, "layer", 404)
} // namespace reflection

} // namespace trillek

#endif
