#include "resources/Material.hpp"
#include "resources/Shader.hpp"


namespace trillek {
namespace resource {
Material::Material() { }
Material::~Material() { }

void Material::SetShader(std::shared_ptr<resource::Shader> s) {
    this->shader = s;
}

size_t Material::AddTexture(std::shared_ptr<resource::Texture> t) {
    for (size_t i = 0; i < this->textures.size(); ++i) {
        if (this->textures[i].first == t) {
            return i;
        }
    }

    // TODO: Generate the texture ID and copy the pixel data
    GLuint texID = 0;

    // The texture wasn't found in the list so add it.
    this->textures.push_back(std::make_pair(t, texID));

    return this->textures.size();
}

size_t Material::GetTextureIndex(std::shared_ptr<resource::Texture> t) {
    for (size_t i = 0; i < this->textures.size(); ++i) {
        if (this->textures[i].first == t) {
            return i;
        }
    }

    return AddTexture(t);
}

void Material::ActivateTexture(const size_t index, const TEXTURE_TARGET target) {
    if (index < this->textures.size()) {
        GLuint texID = this->textures[index].second;
        glActiveTexture(target);
        glBindTexture(GL_TEXTURE_2D, texID);
    }
}
}
}
