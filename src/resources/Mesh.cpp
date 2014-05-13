#include "resources/Mesh.hpp"

namespace trillek {
namespace resource {

std::weak_ptr<MeshGroup> Mesh::GetMeshGroup(const unsigned int& meshIndex) {
    if (meshIndex < this->mesh_groups.size()) {
        return this->mesh_groups[meshIndex];
    }

    return std::weak_ptr<MeshGroup>();
}

} // End of reousrce
} // End of trillek
