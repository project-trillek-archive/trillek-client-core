#ifndef MESH_HPP_INCLUDED
#define MESH_HPP_INCLUDED

#include <glm/glm.hpp>

#include "systems/resource-system.hpp"

namespace trillek {
namespace resource {

// Vertex data used for rendering or other purposes.
struct VertexData {
    VertexData() : position(0.0f, 0.0f, 0.0f), color(0.0f, 1.0f, 0.0f, 1.0f) { }
    glm::vec3 position;
    glm::vec4 color;
    glm::vec3 normal;
    glm::vec2 uv;
};

// Container for holding sub-mesh groups.
struct MeshGroup {
    std::vector<VertexData> verts;
    std::vector<unsigned int> indicies;
};

class Mesh : public ResourceBase {
public:
    Mesh() { }
    ~Mesh() { }

    /**
    * \brief Returns a sub-mesh group.
    *
    * \param[in] const unsigned int& meshGroup The index of the mesh group to retrieve.
    * \return std::weak_ptr<MeshGroup> The mesh group requested
    */
    std::weak_ptr<MeshGroup> GetMeshGroup(const unsigned int& meshGroup);

    /**
    * \brief Returns the number of sub-mesh groups.
    *
    * \return size_t The number of sub-mesh groups.
    */
    size_t GetMeshGroupCount() const {
        return this->mesh_groups.size();
    }
protected:
    std::vector<std::shared_ptr<MeshGroup>> mesh_groups;
};

} // End of resource
} // End of trillek

#endif
