#ifndef RENDERABLE_HPP_INCLUDED
#define RENDERABLE_HPP_INCLUDED

#include "opengl.hpp"
#include "type-id.hpp"
#include <memory>
#include <vector>
#include "component.hpp"

namespace trillek {
namespace resource {

class Mesh;

} // End of resource

namespace graphics {

class Shader;
class Texture;
class Animation;

class Renderable : public ComponentBase {
public:
    Renderable();
    ~Renderable();

    // Helper structure to group buffer IDs
    struct BufferGroup {
        GLuint vao;
        GLuint vbo;
        GLuint ibo;
        unsigned int ibo_count;
        std::vector<std::shared_ptr<Texture>> textures;
    };

    /**
     * \brief Updates (or creates) the Renderable's BufferGroups.
     *
     * This will take the buffers from the mesh resource mesh groups and put
     * them in OpenGL buffers. This will create new buffer groups if needed.
     * \return void
     */
    void UpdateBufferGroups();

    /**
     * \brief Sets the mesh resource associated with this component.
     *
     * \param std::shared_ptr<resource::Mesh> m The mesh resource for this component.
     * \return void
     */
    void SetMesh(std::shared_ptr<resource::Mesh> m);

    /**
     * \brief Gets the mesh resource associated with this component.
     *
     * \return std::shared_ptr<resource::Mesh> The mesh resource for this component.
     */
    std::shared_ptr<resource::Mesh> GetMesh() const;

    /**
     * \brief Sets the shader resource associated with this component.
     *
     * \param std::shared_ptr<Material> m The shader for this component.
     * \return void
     */
    void SetShader(std::shared_ptr<Shader> s);

    /**
     * \brief Gets the shader for this renderable
     *
     * \return std::shared_ptr<Material> This shader for this component.
     */
    std::shared_ptr<Shader> GetShader() const;

    /**
     * \brief Sets the animation for this component.
     *
     * \param[in] const std::shared_ptr<Animation> a The animation for this component.
     * \return void
     */
    void SetAnimation(std::shared_ptr<Animation> a);

    /**
     * \brief Gets the animation for this component.
     *
     * \return const std::shared_ptr<Animation> The animation for this component.
     */
    std::shared_ptr<Animation> GetAnimation() const;

    /**
     * \brief Gets the number of buffer groups.
     *
     * \return size_t The number of buffer groups.
     */
    size_t GetBufferGroupCount() const {
        return this->buffer_groups.size();
    }

    /**
     * \brief Gets the specific buffer group.
     *
     * \param const size_t index The index of the buffer group to return.
     * \return std::shared_ptr<BufferGroup> The requested buffer group.
     */
    std::shared_ptr<BufferGroup> GetBufferGroup(const size_t index) const {
        if (index < this->buffer_groups.size()) {
            return this->buffer_groups[index];
        }

        return nullptr;
    }

    /**
     * \brief Initializes the component with the provided properties
     *
     * Valid properties include mesh (the mesh resource name) and shader (the shader resource name).
     * \param[in] const std::vector<Property>& properties The creation properties for the component.
     * \return bool True if initialization finished with no errors.
     */
    bool Initialize(const std::vector<Property> &properties);
private:
    std::vector<std::shared_ptr<BufferGroup>> buffer_groups; // Render buffer ID group

    std::shared_ptr<resource::Mesh> mesh;

    std::shared_ptr<Animation> animation;

    std::shared_ptr<Shader> shader;

    bool dyn_textures; // Wether the textures for this renderable should be updated each frame or not.

    id_t entity_id;
};

} // End of graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NAME(graphics::Renderable, "renderable", 2000)
} // End of reflection

} // End of trillek

#endif
