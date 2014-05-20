#include "systems/json-parser.hpp"

#include "systems/ResourceSystem.h"
#include "systems/transform-system.hpp"

namespace trillek {
namespace system {

void JSONParser::RegisterTypes() {
    RegisterSerializer(ResourceSystem::GetInstance());
    RegisterSerializer(TransformSystem::GetInstance());
}

} // End of system
} // End of trillek
