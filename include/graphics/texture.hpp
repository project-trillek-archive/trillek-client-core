#ifndef TEXTURE_HPP_INCLUDED
#define TEXTURE_HPP_INCLUDED

#include "type-id.hpp"
#include "resources/pixel-buffer.hpp"
#include "opengl.hpp"
#include "graphics-base.hpp"

namespace trillek {
namespace graphics {

class Texture : public GraphicsBase {
public:
    Texture() : texture_id(0) {}
    virtual ~Texture();

    // required to implement
    virtual bool SystemStart(const std::list<Property> &) { return true; }
    virtual bool SystemReset(const std::list<Property> &) { return true; }
    virtual bool Parse(const std::string &object_name, const rapidjson::Value& node) { return false; }
    virtual bool Serialize(rapidjson::Document& document) { return false; }

    /**
     * \brief new texture instance from image
     */
    Texture(const resource::PixelBuffer &);

    // no copying (although it could be done)
    Texture(const Texture &) = delete;
    Texture& operator=(const Texture &) = delete;

    // Move textures
    Texture(Texture &&);
    Texture& operator=(Texture &&);

    /**
     * \brief delete the underlaying GL texture
     */
    void Destroy();

    /**
     * \brief get the ID of the texture
     * note: this method is not const, since GL can modify the texture with the ID
     * \return GLuint the GL texture ID
     */
    GLuint GetID() { return texture_id; }

    /**
     * \brief create a texture from an image
     */
    void Load(const resource::PixelBuffer &);

    /**
     * \brief create a blank texture RGB or RGBA format
     */
    void Generate(GLuint width, GLuint height, bool usealpha);

    /**
     * \brief create a blank depth texture with or without stencil
     */
    void GenerateDepth(GLuint width, GLuint height, bool stencil);

    /**
     * \brief create a blank stencil texture
     */
    void GenerateStencil(GLuint width, GLuint height);

    /**
     * \brief create a blank multisample texture RGB or RGBA format
     */
    void GenerateMultisample(GLuint width, GLuint height, GLuint samples);

    /**
     * \brief create a blank multisample depth texture with or without stencil
     */
    void GenerateMultisampleDepth(GLuint width, GLuint height, GLuint samples, bool stencil);

    /**
     * \brief create a blank multisample stencil texture
     */
    void GenerateMultisampleStencil(GLuint width, GLuint height, GLuint samples);

    bool operator==(const Texture & other) {
        return texture_id == other.texture_id;
    }
    bool Initialize(const std::vector<Property> &properties) { return true; }
protected:
    GLuint texture_id;

};

} // graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NAME(graphics::Texture,"texture",400)
} // End of reflection

} // trillek

#endif
