#ifndef MATERIAL_HPP_INCLUDED
#define MATERIAL_HPP_INCLUDED

#include "opengl.hpp"

#include <memory>
#include <vector>

namespace trillek {
namespace graphics {

class Texture;
class Shader;

class Material {
public:
    Material();
    ~Material();

    /**
     * \brief Sets the shader used for this material.
     *
     * \param std::shared_ptr<resource::Shader> s The material's shader.
     * \return void
     */
    void SetShader(std::shared_ptr<Shader> s);

    /**
     * \brief Get the material's shader.
     *
     * \return std::shared_ptr<resource::Shader> The material's shader.
     */
    std::shared_ptr<Shader> GetShader() const {
        return this->shader;
    }

    /**
     * \brief Adds a texture to textures and returns its index.
     *
     * \param std::shared_ptr<Texture> t The texture to add.
     * \return size_t index The index of the texture in textures;
     */
    size_t AddTexture(std::shared_ptr<Texture> t);


    /**
     * \brief Get the texture index in texture of the give texture.
     *
     * If the texture isn't found it is added to textures.
     * \param std::shared_ptr<Texture> t The texture to find or add.
     * \return size_t index The index of the texture in textures;
     */
    size_t GetTextureIndex(std::shared_ptr<Texture> t);

    /**
     * \brief Set's the target texture unit as active and binds the texture
     * at index in textures.
     *
     * \param size_t index The index of the texture to bind.
     * \param GLuint target Target texture unit to make active.
     * \return void
     */
    void ActivateTexture(const size_t index, const GLuint target);

    /**
     * \brief Deactivates the specified texture unit.
     *
     * \param GLuint target Target texture unit to deactivate.
     */
    static void DeactivateTexture(GLuint target);
private:
    // Stores a mapping of texture to GL texture ID.
    // During rendering a change is done via the vector index.
    std::vector<std::pair<std::shared_ptr<Texture>, GLuint>> textures;

    std::shared_ptr<Shader> shader;
};

} // End of graphics
} // End of trillek

#endif
