#include "systems/ResourceSystem.h"

namespace trillek {
    namespace resource {
        std::once_flag ResourceSystem::only_one;
        std::shared_ptr<ResourceSystem> ResourceSystem::instance = nullptr;
    }
}
