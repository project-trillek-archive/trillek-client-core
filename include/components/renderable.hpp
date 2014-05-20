#ifndef RENDERABLE_HPP_INCLUDED
#define RENDERABLE_HPP_INCLUDED

#ifndef __APPLE__
#include <GL/glew.h>
#else
#include <OpenGL/gl3.h>
#endif

#include <memory>
#include <vector>

namespace trillek {
namespace resource {

class Mesh;

} // End of resource

namespace graphics {

class Material;

class Renderable {
public:
    Renderable();
    ~Renderable();

    // Helper structure to group buffer IDs
    struct BufferGroup {
        GLuint vao;
        GLuint vbo;
        GLuint ibo;
        unsigned int ibo_count;
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
    * \param std::shared_ptr<resource::Mesh> m The mesh reousrce for this component.
    * \return void
    */
    void SetMesh(std::shared_ptr<resource::Mesh> m);

    /**
    * \brief Gets the mesh resource associated with this component.
    *
    * \return std::shared_ptr<resource::Mesh> The mesh reousrce for this component.
    */
    std::shared_ptr<resource::Mesh> GetMesh() const;

    /**
    * \brief Sets the shader resource associated with this component.
    *
    * \param std::shared_ptr<Material> m The material for this component.
    * \return void
    */
    void SetMaterial(std::shared_ptr<Material> s);

    /**
    * \brief Gets the material for this renderable
    *
    * \return std::shared_ptr<Material> This material for this component.
    */
    std::shared_ptr<Material> GetMaterial() const;

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
        return this->buffer_groups[index];
    }
private:
    std::vector<std::shared_ptr<BufferGroup>> buffer_groups; // Render buffer ID group

    std::shared_ptr<resource::Mesh> mesh;

    std::shared_ptr<Material> material;
};

} // End of graphics
} // End of trillek

#endif
