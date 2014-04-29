#include "resources/Mesh.hpp"

namespace trillek {
    namespace resource {

    std::weak_ptr<MeshGroup> Mesh::GetMeshGroup(const unsigned int& meshIndex) {
        if (meshIndex < this->meshGroups.size()) {
            return this->meshGroups[meshIndex];
        }

        return std::weak_ptr<MeshGroup>();
    }
}}
