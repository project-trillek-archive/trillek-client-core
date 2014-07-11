
#include "graphics/texture.hpp"

namespace trillek {
namespace graphics {

Texture::~Texture() {
    Destroy();
}

Texture::Texture(const resource::PixelBuffer & image) {
    texture_id = 0;
    Load(image);
}

Texture::Texture(Texture && other) {
    texture_id = other.texture_id;
    other.texture_id = 0;
}

Texture& Texture::operator=(Texture && other) {
    texture_id = other.texture_id;
    other.texture_id = 0;
    return *this;
}

void Texture::Destroy() {
    if(texture_id) {
        glDeleteTextures(1, &texture_id);
    }
}

void Texture::Load(const resource::PixelBuffer & image) {
    CheckGLError();
    using resource::ImageColorMode;
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    GLenum gformat;
    switch(image.GetFormat()) {
    case ImageColorMode::COLOR_RGBA:
        gformat = GL_RGBA;
        break;
    case ImageColorMode::COLOR_RGB:
        gformat = GL_RGB;
        break;
    case ImageColorMode::MONOCHROME_A:
        gformat = GL_RG;
        break;
    case ImageColorMode::MONOCHROME:
        gformat = GL_RED;
        break;
    default:
        return;
    }
    const uint8_t * pixdata = image.GetBlockBase();
    if(nullptr == pixdata) {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CheckGLError();
    glTexImage2D(GL_TEXTURE_2D, 0, gformat, image.Width(), image.Height(), 0, gformat, GL_UNSIGNED_BYTE, pixdata);
    CheckGLError();
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture::Generate(GLuint width, GLuint height, bool usealpha) {
    CheckGLError();
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CheckGLError();
    if(usealpha) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        CheckGLError();
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        CheckGLError();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateStencil(GLuint width, GLuint height) {
    CheckGLError();
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CheckGLError();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, width, height, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, nullptr);
    CheckGLError();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateDepth(GLuint width, GLuint height, bool stencil) {
    CheckGLError();
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CheckGLError();
    if(stencil) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, nullptr);
        CheckGLError();
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
        CheckGLError();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateMultisample(GLuint width, GLuint height, GLuint samples) {
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id);
    CheckGLError();
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, width, height, GL_FALSE);
    CheckGLError();

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void Texture::GenerateMultisampleStencil(GLuint width, GLuint height, GLuint samples) {
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id);
    CheckGLError();
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_STENCIL_INDEX, width, height, GL_FALSE);
    CheckGLError();

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void Texture::GenerateMultisampleDepth(GLuint width, GLuint height, GLuint samples, bool stencil) {
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id);
    CheckGLError();
    if(stencil) {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH_STENCIL, width, height, GL_FALSE);
        CheckGLError();
    }
    else {
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH_COMPONENT, width, height, GL_FALSE);
        CheckGLError();
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

} // graphics
} // trillek
