#include "systems/transform_system.hpp"
#include "resources/transform.hpp"

namespace trillek {
namespace transform {

std::once_flag TransformSystem::only_one;
std::shared_ptr<TransformSystem> TransformSystem::instance = nullptr;

std::shared_ptr<resource::Transform> TransformSystem::GetTransform(const unsigned int entity_id) {
    if (instance->transforms.find(entity_id) != instance->transforms.end()) {
        return instance->transforms[entity_id];
    }

    return nullptr;
}

std::shared_ptr<resource::Transform> TransformSystem::AddTransform(const unsigned int entity_id) {
    if (instance->transforms.find(entity_id) == instance->transforms.end()) {
        std::shared_ptr<resource::Transform> transform = std::make_shared<resource::Transform>();

        instance->transforms[entity_id] = transform;
    }

    return instance->transforms[entity_id];
}

void TransformSystem::RemoveTransform(const unsigned int entity_id) {
    instance->transforms.erase(entity_id);
}

} // End of transofmr
} // End of trillek
